// Make watch switch time every 5 seconds
#define DEBUG 0

// Data keys
#define KEY_INVERSE 0
#define KEY_BACKGROUND 1
#define KEY_REGULAR_TEXT 2
#define KEY_BOLD_TEXT 3
#define KEY_LANGUAGE 4
#define KEY_OFFSET 5
#define KEY_GESTURE 7
#define KEY_BT_NOTIFICATION 8
#define KEY_MEETING_STATUS 9
#define KEY_STRICT_HOUR_PHRASES 10
#define KEY_WEATHER_CODE 11
#define KEY_WEATHER_TEMP_F 12

// Max number of characters in a line
#define LINE_LENGTH 9
// How many characters must be available in a line to consider adding a second word
#define LINE_APPEND_MARGIN 2
// We can add a new word to a line if it is no longer than this
#define LINE_APPEND_LIMIT (LINE_LENGTH - LINE_APPEND_MARGIN + 1)

// Max number of lines
#define NUM_LINES 4
// Size of text buffer for lines
#define BUFFER_SIZE (LINE_LENGTH + 2)
// Initial frame height for line TextLayers; configureLayersForText resets per style.
// Sized to the tallest style (Bitham 42) so allocations are big enough up-front.
#define ROW_HEIGHT 52
// Switch from Bitham 42 to compact fonts (Gothic 28 / Bitham 30 Black) at this line count.
// 3-line phrases at Bitham 42 don't fit the available band on any platform.
#define COMPACT_LAYOUT_MIN_LINES 3
// Share of vertical slack placed above the time block (0–100). 50 = centered; lower = less empty top.
#define TIME_BLOCK_CLUSTER_TOP_SLACK_PERCENT 30

// Text alignment. Can be GTextAlignmentLeft, GTextAlignmentCenter or GTextAlignmentRight
#define TEXT_ALIGN GTextAlignmentCenter

// The time it takes for a layer to slide in or out.
#define ANIMATION_DURATION 400
// Delay between the layers animations, from top to bottom
#define ANIMATION_STAGGER_TIME 150
// Delay from the start of the current layer going out until the next layer slides in
#define ANIMATION_OUT_IN_DELAY 100

// How long to show messages, in seconds
#define BT_LOST_DISPLAY_TIME 12

// How long to wait in seconds between connection lost notification and displaying message
#define CONNECTION_LOST_MARGIN 2

// Gestures
#define GESTURE_OFF  0
#define GESTURE_ANY  4

#define BT_NOTIFY_OFF      0
#define BT_NOTIFY_NO_VIBE  1
#define BT_NOTIFY_ON       2

#define MEETING_STATUS_NONE 0
#define MEETING_STATUS_SOON 1
#define MEETING_STATUS_NOW  2


// Data structures
typedef struct {
	TextLayer *currentLayer;
	TextLayer *nextLayer;
	char lineStr1[BUFFER_SIZE];
	char lineStr2[BUFFER_SIZE];
	PropertyAnimation *animation1;
	PropertyAnimation *animation2;
} Line;

// Functions
void animationStoppedHandler(struct Animation *animation, bool finished, void *context);
void makeAnimationsForLayer(Line *line, int delay);
void updateLayerText(TextLayer* layer, char* text);
void updateLineTo(Line *line, char *value, int delay);
bool needToUpdateLine(Line *line, char *nextValue);
void configureLightLayer(TextLayer *textlayer);
int configureLayersForText(char text[NUM_LINES][BUFFER_SIZE], char format[]);
void string_to_lines(char *str, char lines[NUM_LINES][BUFFER_SIZE], char format[]);
void time_to_lines(int hours, int minutes, struct tm *raw_local,
                   char lines[NUM_LINES][BUFFER_SIZE], char format[]);
void display_message(char *message, int displayTime);
void display_time(struct tm *t, bool force);
void update_status_indicators(void);
void handle_tick(struct tm *tick_time, TimeUnits units_changed);
void init_line(Line* line);
struct tm *get_localtime();
void refresh_time();
void set_offset(int offset);
void set_gesture(int gesture);
void set_bt_lost_notification(int bt_notification);
void set_strict_hour_phrases(bool enabled);
void inbox_received_handler(DictionaryIterator *iter, void *context);
void notify_bt_lost();
void bt_handler(bool connected);
void readPersistedState();
void handle_init();
void destroy_line(Line* line);
void handle_deinit();
