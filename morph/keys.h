// Key codes, mods and actions. These are chosen to match GLFW definitions so that Qt
// key codes can be passed in to morph::Visual. The names mostly match. GLFW_KEY_A is
// equivalent to morph::key::A with the exception of the numeric keys. GLFW_KEY_0 is
// morph::key::n0, GLFW_KEY_1 is morph::key::n1 etc. (because morph::key::1 would be
// illegal).

namespace morph {

    struct key
    {
        // The unknown key
        static constexpr int UNKNOWN          =  -1;

        // Printable keys
        static constexpr int SPACE            =  32;
        static constexpr int APOSTROPHE       =  39;  /* ' */
        static constexpr int COMMA            =  44;  /* , */
        static constexpr int MINUS            =  45;  /* - */
        static constexpr int PERIOD           =  46;  /* . */
        static constexpr int SLASH            =  47;  /* / */
        static constexpr int n0               =  48;
        static constexpr int n1               =  49;
        static constexpr int n2               =  50;
        static constexpr int n3               =  51;
        static constexpr int n4               =  52;
        static constexpr int n5               =  53;
        static constexpr int n6               =  54;
        static constexpr int n7               =  55;
        static constexpr int n8               =  56;
        static constexpr int n9               =  57;
        static constexpr int SEMICOLON        =  59;  /* ; */
        static constexpr int EQUAL            =  61;  /* = */
        static constexpr int A                =  65;
        static constexpr int B                =  66;
        static constexpr int C                =  67;
        static constexpr int D                =  68;
        static constexpr int E                =  69;
        static constexpr int F                =  70;
        static constexpr int G                =  71;
        static constexpr int H                =  72;
        static constexpr int I                =  73;
        static constexpr int J                =  74;
        static constexpr int K                =  75;
        static constexpr int L                =  76;
        static constexpr int M                =  77;
        static constexpr int N                =  78;
        static constexpr int O                =  79;
        static constexpr int P                =  80;
        static constexpr int Q                =  81;
        static constexpr int R                =  82;
        static constexpr int S                =  83;
        static constexpr int T                =  84;
        static constexpr int U                =  85;
        static constexpr int V                =  86;
        static constexpr int W                =  87;
        static constexpr int X                =  88;
        static constexpr int Y                =  89;
        static constexpr int Z                =  90;
        static constexpr int LEFT_BRACKET     =  91;  /* [ */
        static constexpr int BACKSLASH        =  92;  /* \ */
        static constexpr int RIGHT_BRACKET    =  93;  /* ] */
        static constexpr int GRAVE_ACCENT     =  96;  /* ` */
        static constexpr int WORLD_1          =  161; /* non-US #1 */
        static constexpr int WORLD_2          =  162; /* non-US #2 */

        // Function keys
        static constexpr int ESCAPE           =  256;
        static constexpr int ENTER            =  257;
        static constexpr int TAB              =  258;
        static constexpr int BACKSPACE        =  259;
        static constexpr int INSERT           =  260;
        static constexpr int DELETE           =  261;
        static constexpr int RIGHT            =  262;
        static constexpr int LEFT             =  263;
        static constexpr int DOWN             =  264;
        static constexpr int UP               =  265;
        static constexpr int PAGE_UP          =  266;
        static constexpr int PAGE_DOWN        =  267;
        static constexpr int HOME             =  268;
        static constexpr int END              =  269;
        static constexpr int CAPS_LOCK        =  280;
        static constexpr int SCROLL_LOCK      =  281;
        static constexpr int NUM_LOCK         =  282;
        static constexpr int PRINT_SCREEN     =  283;
        static constexpr int PAUSE            =  284;
        static constexpr int F1               =  290;
        static constexpr int F2               =  291;
        static constexpr int F3               =  292;
        static constexpr int F4               =  293;
        static constexpr int F5               =  294;
        static constexpr int F6               =  295;
        static constexpr int F7               =  296;
        static constexpr int F8               =  297;
        static constexpr int F9               =  298;
        static constexpr int F10              =  299;
        static constexpr int F11              =  300;
        static constexpr int F12              =  301;
        static constexpr int F13              =  302;
        static constexpr int F14              =  303;
        static constexpr int F15              =  304;
        static constexpr int F16              =  305;
        static constexpr int F17              =  306;
        static constexpr int F18              =  307;
        static constexpr int F19              =  308;
        static constexpr int F20              =  309;
        static constexpr int F21              =  310;
        static constexpr int F22              =  311;
        static constexpr int F23              =  312;
        static constexpr int F24              =  313;
        static constexpr int F25              =  314;
        static constexpr int KP_0             =  320;
        static constexpr int KP_1             =  321;
        static constexpr int KP_2             =  322;
        static constexpr int KP_3             =  323;
        static constexpr int KP_4             =  324;
        static constexpr int KP_5             =  325;
        static constexpr int KP_6             =  326;
        static constexpr int KP_7             =  327;
        static constexpr int KP_8             =  328;
        static constexpr int KP_9             =  329;
        static constexpr int KP_DECIMAL       =  330;
        static constexpr int KP_DIVIDE        =  331;
        static constexpr int KP_MULTIPLY      =  332;
        static constexpr int KP_SUBTRACT      =  333;
        static constexpr int KP_ADD           =  334;
        static constexpr int KP_ENTER         =  335;
        static constexpr int KP_EQUAL         =  336;
        static constexpr int LEFT_SHIFT       =  340;
        static constexpr int LEFT_CONTROL     =  341;
        static constexpr int LEFT_ALT         =  342;
        static constexpr int LEFT_SUPER       =  343;
        static constexpr int RIGHT_SHIFT      =  344;
        static constexpr int RIGHT_CONTROL    =  345;
        static constexpr int RIGHT_ALT        =  346;
        static constexpr int RIGHT_SUPER      =  347;
        static constexpr int MENU             =  348;
    };


    // Flags copied from GLFW. GLFW_MOD_SHIFT is morph::keymod::SHIFT, etc.
    struct keymod
    {
        //  If this bit is set one or more Shift keys were held down.
        static constexpr int SHIFT          = 0x0001;
        //  If this bit is set one or more Control keys were held down.
        static constexpr int CONTROL        = 0x0002;
        //  If this bit is set one or more Alt keys were held down.
        static constexpr int ALT            = 0x0004;
        //  If this bit is set one or more Super keys were held down.
        static constexpr int SUPER          = 0x0008;
        //  If this bit is set the Caps Lock key is enabled and the @ref
        static constexpr int CAPS_LOCK      = 0x0010;
        //  If this bit is set the Num Lock key is enabled and the a num lock input mode
        //  should be enabled (may not be implemented for morphologica)
        static constexpr int NUM_LOCK       = 0x0020;
    };

    // GLFW_PRESS is morph::keyaction::PRESS etc
    struct keyaction
    {
        static constexpr int RELEASE = 0;
        static constexpr int PRESS   = 1;
        static constexpr int REPEAT  = 2;

    };

    // Mouse buttons. left is really 'primary' and right is 'secondary' because
    // windowing system will switch left and right for left handed mouse mode.
    struct mousebutton
    {
        static constexpr int primary   = 0;
        static constexpr int secondary = 1;
        static constexpr int left      = 0;
        static constexpr int right     = 1;
        static constexpr int middle    = 2;
    };
}
