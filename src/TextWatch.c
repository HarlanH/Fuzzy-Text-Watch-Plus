#include <pebble.h>
#include <ctype.h>
#ifdef PBL_COLOR
  #include "gcolor_definitions.h" // Allows the use of color
#endif

#include "num2words.h"
#include "TextWatch.h"
#include "config_message.h"
#include "weather_format.h"

Window *window;
TextLayer *dayOfMonthLayer;
TextLayer *btStatusLayer;
TextLayer *meetingStatusLayer;
TextLayer *batteryStatusLayer;
char dayOfMonthText[32];
char btStatusText[12];
char meetingStatusText[32];
char batteryStatusText[20];
char weatherPhraseText[32];
char topRowPhraseText[32];
int lastDisplayedDay = -1;
AppTimer *backlightTimer = NULL;
bool btConnected = true;
int batteryPercent = 100;
bool topRowShowWeather = false;
bool bottomLineShowDow = false;
bool lastBottomLineShowDow = false;

Line lines[NUM_LINES];

int currentMinutes;
int currentNLines;

// Corrent color of normal text
GColor8 regularTextColor;
// Corrent color of bold text
GColor8 boldTextColor;

// Time in seconds to add to the current time before calculating which
// 5-minute period we should display
int timeOffset;

// When true (default), do not show :00 / :30 phrases until local time reaches that point
static bool strictHourPhrases = true;

// Variale to keep track of the last minute that we updated the time
// Used to optimise so we only need to run time logic once per minute.
int lastMinute = -1;

static bool messageShowing = false;
static AppTimer *messageTimer = NULL;
static AppTimer *connectionLostTimer = NULL;

// Which gesture to activate date screen
// 0 = off
// 1 = Boxing move (X-axis)
// 2 = flick wrist (Y-axis)
// 3 = Shake up/down (Z-axis)
// 4 = Any shake
int dateGesture = GESTURE_ANY;

// Notify when BT connection is lost?
// 0 = off
// 1 = text and light only
// 2 = on
int bt_lost_notification = BT_NOTIFY_ON;
int meetingStatus = MEETING_STATUS_NONE;

static int16_t s_weather_code = -1;
static int16_t s_weather_temp_f;

// Screen resolution. Set in the init function.
int xres;
int yres;

static ConfigMessageContext config_message_context;
#define STATUS_BAR_HEIGHT 24
#define STATUS_BAR_EDGE_PAD 2
#define STATUS_BAR_LABEL_PAD 6
#define STATUS_BAR_CENTER_MIN_W 40
#define BACKLIGHT_TIMEOUT_MS 3000
// Date row position (original placement — do not change for time-block spacing).
#define DAY_LINE_ORIGIN_Y_OFFSET 30
#define DAY_LINE_LAYER_HEIGHT 28
// Extra vertical gap between the last fuzzy time line and the date row (layout only).
#define TIME_BLOCK_GAP_ABOVE_DATE 12
// Bottom inset for configureLayersForText: date strip + breathing room above it.
#define TIME_BLOCK_BOTTOM_INSET (DAY_LINE_ORIGIN_Y_OFFSET + TIME_BLOCK_GAP_ABOVE_DATE)
#define TIME_BLOCK_Y_BIAS -2

void backlight_off_handler(void *context)
{
	(void)context;
	light_enable(false);
	backlightTimer = NULL;
}

static void message_expired_handler(void *context)
{
	(void)context;
	messageTimer = NULL;
	messageShowing = false;
	display_time(get_localtime(), true);
}

static void connection_lost_check_handler(void *context)
{
	(void)context;
	connectionLostTimer = NULL;
	if (!connection_service_peek_pebble_app_connection()) {
		notify_bt_lost();
	}
}

static bool top_row_has_meeting_phrase(void)
{
	return meetingStatus != MEETING_STATUS_NONE;
}

static bool top_row_has_weather_phrase(void)
{
	return weatherPhraseText[0] != '\0';
}

static void build_top_row_phrase(void)
{
	if (top_row_has_meeting_phrase() && top_row_has_weather_phrase()) {
		const char *selected = topRowShowWeather ? weatherPhraseText : meetingStatusText;
		snprintf(topRowPhraseText, sizeof(topRowPhraseText), "%s", selected);
		return;
	}

	if (top_row_has_meeting_phrase()) {
		topRowShowWeather = false;
		snprintf(topRowPhraseText, sizeof(topRowPhraseText), "%s", meetingStatusText);
		return;
	}

	if (top_row_has_weather_phrase()) {
		topRowShowWeather = true;
		snprintf(topRowPhraseText, sizeof(topRowPhraseText), "%s", weatherPhraseText);
		return;
	}

	topRowPhraseText[0] = '\0';
}

static int status_bar_label_width_px(const char *text, GTextAlignment alignment)
{
	if (text == NULL || text[0] == '\0') {
		return 0;
	}
	GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
	GRect box = GRect(0, 0, 2000, STATUS_BAR_HEIGHT);
	GSize sz = graphics_text_layout_get_content_size(text, font, box, GTextOverflowModeFill, alignment);
	return sz.w;
}

static void layout_status_bar_row(void)
{
	int left_rail = STATUS_BAR_EDGE_PAD;
	if (btStatusText[0] != '\0') {
		left_rail = STATUS_BAR_EDGE_PAD + status_bar_label_width_px(btStatusText, GTextAlignmentLeft) + STATUS_BAR_LABEL_PAD;
	}
	int right_rail = STATUS_BAR_EDGE_PAD;
	if (batteryStatusText[0] != '\0') {
		right_rail = STATUS_BAR_EDGE_PAD + status_bar_label_width_px(batteryStatusText, GTextAlignmentRight) + STATUS_BAR_LABEL_PAD;
	}

	int center_w = xres - left_rail - right_rail;
	if (center_w < STATUS_BAR_CENTER_MIN_W) {
		int deficit = STATUS_BAR_CENTER_MIN_W - center_w;
		int min_rail = STATUS_BAR_EDGE_PAD + 8;
		if (left_rail > min_rail) {
			int take = deficit / 2;
			if (take > left_rail - min_rail) {
				take = left_rail - min_rail;
			}
			left_rail -= take;
			deficit -= take;
		}
		if (deficit > 0 && right_rail > min_rail) {
			int take = deficit;
			if (take > right_rail - min_rail) {
				take = right_rail - min_rail;
			}
			right_rail -= take;
		}
		center_w = xres - left_rail - right_rail;
		if (center_w < STATUS_BAR_CENTER_MIN_W) {
			left_rail = STATUS_BAR_EDGE_PAD;
			right_rail = STATUS_BAR_EDGE_PAD;
			center_w = xres - left_rail - right_rail;
		}
	}

	int bt_w = left_rail - STATUS_BAR_EDGE_PAD;
	if (bt_w < 1) {
		bt_w = 1;
	}
	int bat_w = right_rail - STATUS_BAR_EDGE_PAD;
	if (bat_w < 1) {
		bat_w = 1;
	}

	layer_set_frame(text_layer_get_layer(btStatusLayer),
			GRect(STATUS_BAR_EDGE_PAD, 0, bt_w, STATUS_BAR_HEIGHT));
	layer_set_frame(text_layer_get_layer(batteryStatusLayer),
			GRect(xres - right_rail, 0, bat_w, STATUS_BAR_HEIGHT));
	layer_set_frame(text_layer_get_layer(meetingStatusLayer),
			GRect(left_rail, 0, center_w, STATUS_BAR_HEIGHT));
}

static void update_top_row_phrase_rotation(void)
{
	if (top_row_has_meeting_phrase() && top_row_has_weather_phrase()) {
		topRowShowWeather = !topRowShowWeather;
	}
}

static void cycle_top_row_phrase(void)
{
	update_top_row_phrase_rotation();
	update_status_indicators();
}

static void weather_refresh_from_storage(void)
{
	if (s_weather_code < 0) {
		weatherPhraseText[0] = '\0';
		return;
	}

	weather_format_phrase(weatherPhraseText, sizeof(weatherPhraseText), get_current_language_id(), s_weather_code,
			      s_weather_temp_f);
	for (int i = (int)strlen(weatherPhraseText) - 1; i >= 0; i--) {
		if (weatherPhraseText[i] != ' ') {
			break;
		}
		weatherPhraseText[i] = '\0';
	}
}

static void weather_apply_from_app_message(int32_t code, int32_t temp_f)
{
	if (code < 0) {
		s_weather_code = -1;
		weatherPhraseText[0] = '\0';
		return;
	}

	s_weather_code = (int16_t)code;
	s_weather_temp_f = (int16_t)temp_f;
	weather_refresh_from_storage();
}

void update_status_indicators(void)
{
	bool show_bt = !btConnected;
	bool show_shh = quiet_time_is_active();
	if (show_bt && show_shh) {
		snprintf(btStatusText, sizeof(btStatusText), "BT! SHH");
	} else if (show_bt) {
		snprintf(btStatusText, sizeof(btStatusText), "BT!");
	} else if (show_shh) {
		snprintf(btStatusText, sizeof(btStatusText), "SHH");
	} else {
		btStatusText[0] = '\0';
	}

	if (batteryPercent < 40) {
		snprintf(batteryStatusText, sizeof(batteryStatusText), "%s", get_battery_low_label());
	} else {
		batteryStatusText[0] = '\0';
	}

	if (meetingStatus == MEETING_STATUS_NOW) {
		get_meeting_now_message(meetingStatusText, sizeof(meetingStatusText));
	} else if (meetingStatus == MEETING_STATUS_SOON) {
		get_meeting_soon_message(meetingStatusText, sizeof(meetingStatusText));
	} else {
		meetingStatusText[0] = '\0';
	}
	build_top_row_phrase();

	text_layer_set_text(btStatusLayer, btStatusText);
	text_layer_set_text(meetingStatusLayer, topRowPhraseText);
	text_layer_set_text(batteryStatusLayer, batteryStatusText);

	layout_status_bar_row();
}

static void apply_current_palette_to_layers(void)
{
	text_layer_set_text_color(dayOfMonthLayer, regularTextColor);
	text_layer_set_text_color(btStatusLayer, regularTextColor);
	text_layer_set_text_color(meetingStatusLayer, regularTextColor);
	text_layer_set_text_color(batteryStatusLayer, regularTextColor);
}

// UTF8 aware strlen() for a sequence of bytes
int strlenUtf8(char *start, char *end) 
{
	int span = end - start;
	int i = 0, j = 0;
	while (start[i] && i < span) 
	{
		if ((start[i] & 0xc0) != 0x80) j++;
		i++;
	}
	return j;
}

// Animation handler
void animationStoppedHandler(struct Animation *animation, bool finished, void *context)
{
	TextLayer *current = (TextLayer *)context;
	GRect rect = layer_get_frame((Layer *)current);
	rect.origin.x = xres;
	layer_set_frame((Layer *)current, rect);
}

// Animate line
void makeAnimationsForLayer(Line *line, int delay)
{
	TextLayer *current = line->currentLayer;
	TextLayer *next = line->nextLayer;

#ifdef PBL_PLATFORM_APLITE
	// Destroy old animations 
	if (line->animation1 != NULL)
	{
		 property_animation_destroy(line->animation1);
	}
	if (line->animation2 != NULL)
	{
		 property_animation_destroy(line->animation2);
	}
#endif

	// Configure animation for current layer to move out
	GRect rect = layer_get_frame((Layer *)current);
	rect.origin.x =  -xres;
	line->animation1 = property_animation_create_layer_frame((Layer *)current, NULL, &rect);
	Animation *animation = property_animation_get_animation(line->animation1);
	animation_set_duration(animation, ANIMATION_DURATION);
	animation_set_delay(animation, delay);
	animation_set_curve(animation, AnimationCurveEaseIn); // Accelerate

	// Configure animation for current layer to move in
	GRect rect2 = layer_get_frame((Layer *)next);
	rect2.origin.x = 0;
	line->animation2 = property_animation_create_layer_frame((Layer *)next, NULL, &rect2);
	animation = property_animation_get_animation(line->animation2);	
	animation_set_duration(animation, ANIMATION_DURATION);
	animation_set_delay(animation, delay + ANIMATION_OUT_IN_DELAY);
	animation_set_curve(animation, AnimationCurveEaseOut); // Deaccelerate

	// Set a handler to rearrange layers after animation is finished
	animation_set_handlers(animation, (AnimationHandlers) {
		.stopped = (AnimationStoppedHandler)animationStoppedHandler
	}, current);

	// Start the animations
	animation_schedule(property_animation_get_animation(line->animation1));
	animation_schedule(property_animation_get_animation(line->animation2));	
}

void updateLayerText(TextLayer* layer, char* text)
{
	const char* layerText = text_layer_get_text(layer);
	strcpy((char*)layerText, text);
	// To mark layer dirty
	text_layer_set_text(layer, layerText);
    //layer_mark_dirty(&layer->layer);
}

// Update line
void updateLineTo(Line *line, char *value, int delay)
{
	updateLayerText(line->nextLayer, value);
	makeAnimationsForLayer(line, delay);

	// Swap current/next layers
	TextLayer *tmp = line->nextLayer;
	line->nextLayer = line->currentLayer;
	line->currentLayer = tmp;
}

// Check to see if the current line needs to be updated
bool needToUpdateLine(Line *line, char *nextValue)
{
	const char *currentStr = text_layer_get_text(line->currentLayer);

	if (strcmp(currentStr, nextValue) != 0) {
		return true;
	}
	return false;
}

// Font for a line from string_to_lines format + whether we use the 4-line compact style.
static GFont font_for_line_format(char format_char, bool compactLayout)
{
	if (format_char == 'B') {
		(void)compactLayout;
		return fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
	}
	if (format_char == 'b') {
		return fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK);
	}
	if (format_char == 's') {
		return fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
	}
	// normal: ' ' or any other
	return compactLayout
		? fonts_get_system_font(FONT_KEY_GOTHIC_28)
		: fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT);
}

static void configure_line_for_format(TextLayer *textlayer, char format_char, bool compactLayout)
{
	GFont font = font_for_line_format(format_char, compactLayout);
	text_layer_set_font(textlayer, font);
	if (format_char == 'B' || format_char == 'b') {
		text_layer_set_text_color(textlayer, boldTextColor);
	} else {
		text_layer_set_text_color(textlayer, regularTextColor);
	}
	text_layer_set_background_color(textlayer, GColorClear);
	text_layer_set_text_alignment(textlayer, TEXT_ALIGN);
}

// Default style for new line layers (wide non-compact light)
void configureLightLayer(TextLayer *textlayer)
{
	configure_line_for_format(textlayer, ' ', false);
}

/*
 * Per-style geometry for one rendered line.
 *
 *   frame_h:   height of the TextLayer frame for this style. Must be >= the font's
 *              full glyph box (ascender + descender + a pixel or two of safety) so
 *              the TextLayer never clips a descender. Independent of which glyphs
 *              actually appear in the line.
 *
 *   line_step: y-advance from this line's top to the next line's top. Typically
 *              smaller than frame_h: the empty space below one line's descender
 *              can overlap the next line's leading without visual collision,
 *              which is how lines pack tightly without clipping.
 *
 * Values are empirically chosen for the Pebble system fonts. They do not depend
 * on the text in the line, so the layout is fully deterministic.
 */
typedef struct {
	int frame_h;
	int line_step;
} LineStyleMetrics;

static LineStyleMetrics line_style_metrics(char format_char, bool compactLayout)
{
	// Bitham 42 (Light/Bold): cap+desc ~= 50 px; 52 px frame leaves headroom.
	// 44 px step gives a tight but readable two-line block.
	const LineStyleMetrics BITHAM_42 = { .frame_h = 52, .line_step = 44 };

	// Bitham 30 Black: full glyph box ~= 34 px; 36 frame, 30 step.
	const LineStyleMetrics BITHAM_30_BLACK = { .frame_h = 36, .line_step = 30 };

	// Gothic 28 (Regular/Bold): full glyph box ~= 30 px; 32 frame, 26 step.
	// Step is intentionally tight so 4-line layouts fit the available band.
	const LineStyleMetrics GOTHIC_28 = { .frame_h = 32, .line_step = 26 };

	if (format_char == 'B') {
		(void)compactLayout;
		return BITHAM_42;
	}
	if (format_char == 'b') {
		return BITHAM_30_BLACK;
	}
	if (format_char == 's') {
		return GOTHIC_28;
	}
	return compactLayout ? GOTHIC_28 : BITHAM_42;
}

/*
 * Configure the line layers for the given text and return the number of lines used.
 *
 * Algorithm:
 *   1. Count non-empty lines; pick the compact font set when there are 3+ lines
 *      (Bitham 42 doesn't fit 3 stacked lines on any platform).
 *   2. For each line, look up its (frame_h, line_step) from the metrics table.
 *      No runtime text measurement; same input always produces the same output.
 *   3. Block height = sum of line_steps + the last line's frame_h. Center the
 *      block in the band between the status bar and the day-of-month strip,
 *      biased slightly upward via TIME_BLOCK_CLUSTER_TOP_SLACK_PERCENT.
 *   4. Place each line at ypos with its frame_h; advance ypos by line_step.
 *
 * Because frame_h is always >= the font's full glyph box, TextLayer cannot clip
 * a descender no matter what string lands in the line.
 */
int configureLayersForText(char text[NUM_LINES][BUFFER_SIZE], char format[])
{
	int numLines = 0;
	for (int i = 0; i < NUM_LINES; i++) {
		if (strlen(text[i]) == 0) {
			break;
		}
		numLines++;
	}
	if (numLines == 0) {
		return 0;
	}

	bool compactLayout = (numLines >= COMPACT_LAYOUT_MIN_LINES);

	LineStyleMetrics styles[NUM_LINES];
	int block_h = 0;
	for (int i = 0; i < numLines; i++) {
		configure_line_for_format(lines[i].nextLayer, format[i], compactLayout);
		styles[i] = line_style_metrics(format[i], compactLayout);
		block_h += (i == numLines - 1) ? styles[i].frame_h : styles[i].line_step;
	}

	int topInset = STATUS_BAR_HEIGHT + 2;
	int bottomInset = TIME_BLOCK_BOTTOM_INSET;
	int availableHeight = yres - topInset - bottomInset;

	int slack = availableHeight - block_h;
	int ypos;
	if (slack > 0) {
		int top_slack = (slack * TIME_BLOCK_CLUSTER_TOP_SLACK_PERCENT) / 100;
		ypos = topInset + TIME_BLOCK_Y_BIAS + top_slack;
	} else {
		// Block taller than the band (4-line edge case): hug the top inset and
		// let the natural line_step values keep the bottom line clear of the date row.
		ypos = topInset + TIME_BLOCK_Y_BIAS + slack / 2;
		if (ypos < topInset) {
			ypos = topInset;
		}
	}

	for (int j = 0; j < numLines; j++) {
		// Visible line at x=0; spare buffer off the right (x=xres). The two
		// TextLayers swap roles each tick to drive the slide-in/out animation.
		GRect visible = GRect(0, ypos, xres, styles[j].frame_h);
		GRect off_right = GRect(xres, ypos, xres, styles[j].frame_h);
		layer_set_frame((Layer *)lines[j].currentLayer, visible);
		layer_set_frame((Layer *)lines[j].nextLayer, off_right);
		if (j < numLines - 1) {
			ypos += styles[j].line_step;
		}
	}

	return numLines;
}

void string_to_lines(char *str, char lines[NUM_LINES][BUFFER_SIZE], char format[])
{
	char *start = str;
	char *end = strstr(start, " ");
	int l = 0;

	// Empty all lines
	for (int i = 0; i < NUM_LINES; i++)
	{
		lines[i][0] = '\0';
	}

	while (end != NULL && l < NUM_LINES) {
		// Check word for bold prefix
		if (*start == '*' && end - start > 1)
		{
			// Mark line bold and move start to the first character of the word
			format[l] = 'B';
			start++;
			if (*start == '<') { // small bold, actually
				format[l] = 'b';
				start++;
			}
		}
		else if (*start == '<' && end - start > 1) {
			// Mark line small
			format[l] = 's';
			start++;
		} else {
			// Mark line normal
			format[l] = ' ';
		}

		char *nextWord = end + 1;

		// Can we add another word to the line?
		if (format[l] == ' ' && *nextWord != '*'  // are both lines formatted normal?
			&& *nextWord != ' '                   // no no-join annotation (double space)
			&& strlenUtf8(start, end) < LINE_APPEND_LIMIT)  // is the first word short enough?
		{
			// See if next word fits
			char *try = strstr(end + 1, " ");
			if (try != NULL
				&& strlenUtf8(start, try) <= LINE_LENGTH)
			{
				end = try;
			}
		}

		// copy to line
		*end = '\0';
		strcpy(lines[l++], start);

		// Look for next word
		start = end + 1;
		while (*start == ' ') start++; // Skip all spaces
		end = strstr(start, " ");
	}
}

void time_to_lines(int hours, int minutes, struct tm *raw_local,
                   char lines[NUM_LINES][BUFFER_SIZE], char format[])
{
	char timeStr[NUM_LINES * BUFFER_SIZE + 1];
	time_to_words(hours, minutes, timeStr, sizeof(timeStr), strictHourPhrases, raw_local);

	string_to_lines(timeStr, lines, format);
}

void day_to_ordinal_words(int day, char *output, size_t length)
{
	get_day_of_month_message(day, output, length);
}

void update_bottom_line(struct tm *t, bool force)
{
	if (!force && lastDisplayedDay == t->tm_mday && bottomLineShowDow == lastBottomLineShowDow) {
		return;
	}

	lastDisplayedDay = t->tm_mday;
	lastBottomLineShowDow = bottomLineShowDow;

	if (bottomLineShowDow) {
		strftime(dayOfMonthText, sizeof(dayOfMonthText), "%A", t);
		for (char *p = dayOfMonthText; *p; p++) {
			*p = tolower((unsigned char)*p);
		}
	} else {
		day_to_ordinal_words(t->tm_mday, dayOfMonthText, sizeof(dayOfMonthText));
	}
	text_layer_set_text(dayOfMonthLayer, dayOfMonthText);
}

// Update screen based on new time
void display_message(char *message, int displayTime)
{
	if (displayTime > 0 && !messageShowing) {
		char textLine[NUM_LINES][BUFFER_SIZE];
		char format[NUM_LINES];

		string_to_lines(message, textLine, format);
		
		int nextNLines = configureLayersForText(textLine, format);

		int delay = 0;
		for (int i = 0; i < NUM_LINES; i++) {
		    updateLineTo(&lines[i], textLine[i], delay);
		    delay += ANIMATION_STAGGER_TIME;
		}
		
		currentNLines = nextNLines;
		messageShowing = true;
		messageTimer = app_timer_register(displayTime * 1000, message_expired_handler, NULL);
	}
}

// Update screen based on new time
void display_time(struct tm *t, bool force)
{
	if (messageShowing) {
		return;
	}

	struct tm raw_tm = *t;
	time_t timestamp = mktime(&raw_tm);
	timestamp += timeOffset; // Add offset time
	struct tm *adj_ptr = localtime(&timestamp);
	struct tm adj_tm = *adj_ptr;

#if DEBUG == 0
	if (lastMinute == adj_tm.tm_min && !force) { // No change in time
		return;
	}
#endif

	// Mark this minute as checked;
	lastMinute = adj_tm.tm_min;

	// The current time text will be stored in the following strings
	char textLine[NUM_LINES][BUFFER_SIZE];
	char format[NUM_LINES];

#if DEBUG == 1
	time_to_lines(adj_tm.tm_hour, adj_tm.tm_sec, NULL, textLine, format);
#else
	time_to_lines(adj_tm.tm_hour, adj_tm.tm_min, &raw_tm, textLine, format);
#endif
	
	int nextNLines = configureLayersForText(textLine, format);

	int delay = 0;
	for (int i = 0; i < NUM_LINES; i++) {
		if (force || nextNLines != currentNLines || needToUpdateLine(&lines[i], textLine[i])) {
			updateLineTo(&lines[i], textLine[i], delay);
			delay += ANIMATION_STAGGER_TIME;
		}
	}
	
	currentNLines = nextNLines;
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Tap event: %d  conf: %d", axis, dateGesture);

  if (dateGesture != GESTURE_ANY) {
  	if (axis == ACCEL_AXIS_X && dateGesture != GESTURE_X) {
	  return;
	}
  	if (axis == ACCEL_AXIS_Y && dateGesture != GESTURE_Y) {
	  return;
	}
  	if (axis == ACCEL_AXIS_Z && dateGesture != GESTURE_Z) {
	  return;
	}
  }

  if (backlightTimer != NULL) {
   	cycle_top_row_phrase();
   	bottomLineShowDow = !bottomLineShowDow;
   	update_bottom_line(get_localtime(), true);
   	app_timer_cancel(backlightTimer);
   }

  light_enable(true);
  backlightTimer = app_timer_register(BACKLIGHT_TIMEOUT_MS, backlight_off_handler, NULL);
}

void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  update_top_row_phrase_rotation();
  bottomLineShowDow = !bottomLineShowDow;

  update_status_indicators();
  update_bottom_line(tick_time, false);
  display_time(tick_time, false);
}

void init_line(Line* line) {
	// Create layers with dummy position to the right of the screen
	line->currentLayer = text_layer_create(GRect(xres, 0, xres, ROW_HEIGHT));
	line->nextLayer = text_layer_create(GRect(xres, 0, xres, ROW_HEIGHT));

	// Configure a style
	configureLightLayer(line->currentLayer);
	configureLightLayer(line->nextLayer);

	// Set the text buffers
	line->lineStr1[0] = '\0';
	line->lineStr2[0] = '\0';
	text_layer_set_text(line->currentLayer, line->lineStr1);
	text_layer_set_text(line->nextLayer, line->lineStr2);

	// Initially there are no animations
	line->animation1 = NULL;
	line->animation2 = NULL;
}

struct tm *get_localtime()
{
	time_t raw_time;
	time(&raw_time);
	return localtime(&raw_time);
}

void refresh_time() {
	apply_current_palette_to_layers();
	weather_refresh_from_storage();
	update_status_indicators();
	update_bottom_line(get_localtime(), true);
	display_time(get_localtime(), true);
}

void set_offset(int offset) {
	timeOffset = offset;
}

void set_gesture(int gesture) {
	dateGesture = gesture;
	accel_tap_service_unsubscribe();
	if (gesture != GESTURE_OFF) {
		accel_tap_service_subscribe(accel_tap_handler);
	}
}

void set_bt_lost_notification(int bt_notification) {
	bt_lost_notification = bt_notification;
}

void set_strict_hour_phrases(bool enabled) {
	strictHourPhrases = enabled;
}

void set_meeting_status(int status) {
	if (status < MEETING_STATUS_NONE || status > MEETING_STATUS_NOW) {
		return;
	}
	meetingStatus = status;
	update_status_indicators();
}

void inbox_received_handler(DictionaryIterator *iter, void *context) {
  (void)context;
  Tuple *meeting_status_t = dict_find(iter, KEY_MEETING_STATUS);
  if (meeting_status_t) {
  	set_meeting_status(meeting_status_t->value->uint8);
  }
  Tuple *weather_code_t = dict_find(iter, KEY_WEATHER_CODE);
  Tuple *weather_temp_t = dict_find(iter, KEY_WEATHER_TEMP_F);
  if (weather_code_t && weather_temp_t) {
  	weather_apply_from_app_message(weather_code_t->value->int32, weather_temp_t->value->int32);
  	update_status_indicators();
  }
  config_message_handle_inbox(iter, &config_message_context);
}

void notify_bt_lost() {
	if (bt_lost_notification != BT_NOTIFY_OFF) {
		if (bt_lost_notification == BT_NOTIFY_ON && !quiet_time_is_active()) {
			vibes_long_pulse();
		}
		light_enable_interaction();
		char message[48];
		get_connection_lost_message(message);
		display_message(message, BT_LOST_DISPLAY_TIME);
	}
}

void bt_handler(bool connected) {
	btConnected = connected;
	update_status_indicators();

	if (connected) {
		if (connectionLostTimer != NULL) {
			app_timer_cancel(connectionLostTimer);
			connectionLostTimer = NULL;
		}
	} else {
		if (connectionLostTimer != NULL) {
			app_timer_cancel(connectionLostTimer);
		}
		connectionLostTimer = app_timer_register(CONNECTION_LOST_MARGIN * 1000,
						       connection_lost_check_handler, NULL);
	}
}

void battery_handler(BatteryChargeState charge_state) {
	batteryPercent = charge_state.charge_percent;
	update_status_indicators();
}

void readPersistedState() {
	config_message_read_persisted_state(&config_message_context);
}

void handle_init() {
	window = window_create();
	window_stack_push(window, true);

	config_message_context.window = window;
	config_message_context.regular_text_color = &regularTextColor;
	config_message_context.bold_text_color = &boldTextColor;
	config_message_context.set_language = set_language;
	config_message_context.set_offset = set_offset;
	config_message_context.set_gesture = set_gesture;
	config_message_context.set_bt_lost_notification = set_bt_lost_notification;
	config_message_context.set_strict_hour_phrases = set_strict_hour_phrases;
	config_message_context.refresh_time = refresh_time;

	Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);
    xres = window_bounds.size.w;
    yres = window_bounds.size.h;

	dayOfMonthLayer = text_layer_create(
		GRect(0, yres - DAY_LINE_ORIGIN_Y_OFFSET, xres, DAY_LINE_LAYER_HEIGHT));
	text_layer_set_font(dayOfMonthLayer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	text_layer_set_text_color(dayOfMonthLayer, regularTextColor);
	text_layer_set_background_color(dayOfMonthLayer, GColorClear);
	text_layer_set_text_alignment(dayOfMonthLayer, TEXT_ALIGN);
	dayOfMonthText[0] = '\0';
	text_layer_set_text(dayOfMonthLayer, dayOfMonthText);
	layer_add_child(window_layer, text_layer_get_layer(dayOfMonthLayer));

	btStatusLayer = text_layer_create(GRect(2, 0, xres / 2, STATUS_BAR_HEIGHT));
	text_layer_set_font(btStatusLayer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_color(btStatusLayer, regularTextColor);
	text_layer_set_background_color(btStatusLayer, GColorClear);
	text_layer_set_text_alignment(btStatusLayer, GTextAlignmentLeft);
	btStatusText[0] = '\0';
	text_layer_set_text(btStatusLayer, btStatusText);
	layer_add_child(window_layer, text_layer_get_layer(btStatusLayer));

	meetingStatusLayer = text_layer_create(GRect(0, 0, xres, STATUS_BAR_HEIGHT));
	// this isn't where the font is set -- look at text_layer_set_font() in meetingStatus branch
	text_layer_set_font(meetingStatusLayer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_color(meetingStatusLayer, regularTextColor);
	text_layer_set_background_color(meetingStatusLayer, GColorClear);
	text_layer_set_text_alignment(meetingStatusLayer, GTextAlignmentCenter);
	text_layer_set_overflow_mode(meetingStatusLayer, GTextOverflowModeTrailingEllipsis);
	meetingStatusText[0] = '\0';
	text_layer_set_text(meetingStatusLayer, meetingStatusText);
	layer_add_child(window_layer, text_layer_get_layer(meetingStatusLayer));

	batteryStatusLayer = text_layer_create(GRect(xres / 2 - 2, 0, xres / 2, STATUS_BAR_HEIGHT));
	text_layer_set_font(batteryStatusLayer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_color(batteryStatusLayer, regularTextColor);
	text_layer_set_background_color(batteryStatusLayer, GColorClear);
	text_layer_set_text_alignment(batteryStatusLayer, GTextAlignmentRight);
	batteryStatusText[0] = '\0';
	text_layer_set_text(batteryStatusLayer, batteryStatusText);
	layer_add_child(window_layer, text_layer_get_layer(batteryStatusLayer));

    // Subscribe to taps
	accel_tap_service_subscribe(accel_tap_handler);


	readPersistedState();

	// Init and load lines
	for (int i = 0; i < NUM_LINES; i++)
	{
		init_line(&lines[i]);
	  	layer_add_child(window_layer, (Layer *)lines[i].currentLayer);
		layer_add_child(window_layer, (Layer *)lines[i].nextLayer);
	}

	refresh_time();
	// Subscribe to ticks
	tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);

	// Subscribe to bluetooth events
	connection_service_subscribe((ConnectionHandlers) {
	  .pebble_app_connection_handler = bt_handler
	});
	btConnected = connection_service_peek_pebble_app_connection();

	// Subscribe to battery events
	battery_state_service_subscribe(battery_handler);
	batteryPercent = battery_state_service_peek().charge_percent;
	update_status_indicators();

	// Set up listener for configuration changes
	app_message_register_inbox_received(inbox_received_handler);
  	AppMessageResult result = app_message_open(512, 512);
  	if (result != APP_MSG_OK) {
  		APP_LOG(APP_LOG_LEVEL_WARNING, "app_message_open() failed with error %d", result);
  	}
}

void destroy_line(Line* line)
{
	// Free layers
	text_layer_destroy(line->currentLayer);
	text_layer_destroy(line->nextLayer);
}

void handle_deinit()
{
	// Free lines
	for (int i = 0; i < NUM_LINES; i++)
	{
		destroy_line(&lines[i]);
	}
	text_layer_destroy(dayOfMonthLayer);
	text_layer_destroy(btStatusLayer);
	text_layer_destroy(meetingStatusLayer);
	text_layer_destroy(batteryStatusLayer);
	connection_service_unsubscribe();
	battery_state_service_unsubscribe();

	// Free window
	window_destroy(window);
}

int main(void)
{
	handle_init();
	app_event_loop();
	handle_deinit();
}

