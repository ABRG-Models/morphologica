#pragma once

// Key codes, mods and actions. These are chosen to match GLFW definitions so that
// Qt/wx/other key codes can be passed in to morph::Visual. The names mostly match,
// apart from being in lower case. GLFW_KEY_A is equivalent to morph::key::a, etc. The
// numeric keys are slightly different. GLFW_KEY_0 is morph::key::n0, GLFW_KEY_1 is
// morph::key::n1 etc. (because morph::key::1 would be illegal C++ code).

namespace morph {

    struct key
    {
        // The unknown key
        static constexpr int unknown          =  -1;

        // Printable keys
        static constexpr int space            =  32;
        static constexpr int apostrophe       =  39;  /* ' */
        static constexpr int comma            =  44;  /* , */
        static constexpr int minus            =  45;  /* - */
        static constexpr int period           =  46;  /* . */
        static constexpr int slash            =  47;  /* / */
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
        static constexpr int semicolon        =  59;  /* ; */
        static constexpr int equal            =  61;  /* = */
        static constexpr int a                =  65;
        static constexpr int b                =  66;
        static constexpr int c                =  67;
        static constexpr int d                =  68;
        static constexpr int e                =  69;
        static constexpr int f                =  70;
        static constexpr int g                =  71;
        static constexpr int h                =  72;
        static constexpr int i                =  73;
        static constexpr int j                =  74;
        static constexpr int k                =  75;
        static constexpr int l                =  76;
        static constexpr int m                =  77;
        static constexpr int n                =  78;
        static constexpr int o                =  79;
        static constexpr int p                =  80;
        static constexpr int q                =  81;
        static constexpr int r                =  82;
        static constexpr int s                =  83;
        static constexpr int t                =  84;
        static constexpr int u                =  85;
        static constexpr int v                =  86;
        static constexpr int w                =  87;
        static constexpr int x                =  88;
        static constexpr int y                =  89;
        static constexpr int z                =  90;
        static constexpr int left_bracket     =  91;  /* [ */
        static constexpr int backslash        =  92;  /* \ */
        static constexpr int right_bracket    =  93;  /* ] */
        static constexpr int grave_accent     =  96;  /* ` */
        static constexpr int world_1          =  161; /* non-US #1 */
        static constexpr int world_2          =  162; /* non-US #2 */

        // Function keys
        static constexpr int escape           =  256;
        static constexpr int enter            =  257;
        static constexpr int tab              =  258;
        static constexpr int backspace        =  259;
        static constexpr int insert           =  260;
        static constexpr int delete_key       =  261; // can't use delete, a keyword
        static constexpr int right            =  262;
        static constexpr int left             =  263;
        static constexpr int down             =  264;
        static constexpr int up               =  265;
        static constexpr int page_up          =  266;
        static constexpr int page_down        =  267;
        static constexpr int home             =  268;
        static constexpr int end              =  269;
        static constexpr int caps_lock        =  280;
        static constexpr int scroll_lock      =  281;
        static constexpr int num_lock         =  282;
        static constexpr int print_screen     =  283;
        static constexpr int pause            =  284;
        static constexpr int f1               =  290;
        static constexpr int f2               =  291;
        static constexpr int f3               =  292;
        static constexpr int f4               =  293;
        static constexpr int f5               =  294;
        static constexpr int f6               =  295;
        static constexpr int f7               =  296;
        static constexpr int f8               =  297;
        static constexpr int f9               =  298;
        static constexpr int f10              =  299;
        static constexpr int f11              =  300;
        static constexpr int f12              =  301;
        static constexpr int f13              =  302;
        static constexpr int f14              =  303;
        static constexpr int f15              =  304;
        static constexpr int f16              =  305;
        static constexpr int f17              =  306;
        static constexpr int f18              =  307;
        static constexpr int f19              =  308;
        static constexpr int f20              =  309;
        static constexpr int f21              =  310;
        static constexpr int f22              =  311;
        static constexpr int f23              =  312;
        static constexpr int f24              =  313;
        static constexpr int f25              =  314;
        static constexpr int kp_0             =  320;
        static constexpr int kp_1             =  321;
        static constexpr int kp_2             =  322;
        static constexpr int kp_3             =  323;
        static constexpr int kp_4             =  324;
        static constexpr int kp_5             =  325;
        static constexpr int kp_6             =  326;
        static constexpr int kp_7             =  327;
        static constexpr int kp_8             =  328;
        static constexpr int kp_9             =  329;
        static constexpr int kp_decimal       =  330;
        static constexpr int kp_divide        =  331;
        static constexpr int kp_multiply      =  332;
        static constexpr int kp_subtract      =  333;
        static constexpr int kp_add           =  334;
        static constexpr int kp_enter         =  335;
        static constexpr int kp_equal         =  336;
        static constexpr int left_shift       =  340;
        static constexpr int left_control     =  341;
        static constexpr int left_alt         =  342;
        static constexpr int left_super       =  343;
        static constexpr int right_shift      =  344;
        static constexpr int right_control    =  345;
        static constexpr int right_alt        =  346;
        static constexpr int right_super      =  347;
        static constexpr int menu             =  348;
    };


    // Flags copied from GLFW. GLFW_MOD_SHIFT is morph::keymod::SHIFT, etc.
    struct keymod
    {
        //  if this bit is set one or more shift keys were held down.
        static constexpr int shift          = 0x0001;
        //  if this bit is set one or more control keys were held down.
        static constexpr int control        = 0x0002;
        //  if this bit is set one or more alt keys were held down.
        static constexpr int alt            = 0x0004;
        //  if this bit is set one or more super keys were held down.
        static constexpr int super          = 0x0008;
        //  if this bit is set the caps lock key is enabled and the @ref
        static constexpr int caps_lock      = 0x0010;
        //  if this bit is set the num lock key is enabled and the a num lock input mode
        //  should be enabled (may not be implemented for morphologica)
        static constexpr int num_lock       = 0x0020;
    };

    // GLFW_PRESS is morph::keyaction::PRESS etc
    struct keyaction
    {
        static constexpr int release = 0;
        static constexpr int press   = 1;
        static constexpr int repeat  = 2;

    };

    // Mouse buttons. left is really 'primary' and right is 'secondary' because
    // windowing system will switch left and right for left handed mouse mode.
    struct mousebutton
    {
        static constexpr int unhandled = -1;
        static constexpr int primary   = 0;
        static constexpr int secondary = 1;
        static constexpr int left      = 0;
        static constexpr int right     = 1;
        static constexpr int middle    = 2;
    };
}
