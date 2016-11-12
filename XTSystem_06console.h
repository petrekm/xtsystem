

#define enum_16(NAME,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16) \
class NAME \
{ \
public: \
    static const int numItems = 10; \
    typedef enum { T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8),_T(#T9),_T(#T10),_T(#T11),_T(#T12),_T(#T13),_T(#T14),_T(#T15),_T(#T16)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = { T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16 }; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \

namespace XTSystem
{
    enum_16(ConsoleColor, Black , DarkBlue , DarkGreen , DarkCyan , DarkRed , DarkMagenta , DarkYellow , Gray , DarkGray , Blue , Green , Cyan , Red , Magenta , Yellow , White)
    flags_4_manual(ConsoleModifiers, None, 0, Alt, 1, Shift, 2, Control, 4)

    class ConsoleKey
    {
    public:
        static const int numItems = 144;
        typedef enum
        {
            Backspace = 8,
            Tab = 9,
            Clear = 12,
            Enter = 13,
            Pause = 19,
            Escape = 27,
            Spacebar = 32,
            PageUp = 33,
            PageDown = 34,
            End = 35,
            Home = 36,
            LeftArrow = 37,
            UpArrow = 38,
            RightArrow = 39,
            DownArrow = 40,
            Select = 41,
            Print = 42,
            Execute = 43,
            PrintScreen = 44,
            Insert = 45,
            Delete = 46,
            Help = 47,
            D0 = 48,
            D1 = 49,
            D2 = 50,
            D3 = 51,
            D4 = 52,
            D5 = 53,
            D6 = 54,
            D7 = 55,
            D8 = 56,
            D9 = 57,
            A = 65,
            B = 66,
            C = 67,
            D = 68,
            E = 69,
            F = 70,
            G = 71,
            H = 72,
            I = 73,
            J = 74,
            K = 75,
            L = 76,
            M = 77,
            N = 78,
            O = 79,
            P = 80,
            Q = 81,
            R = 82,
            S = 83,
            T = 84,
            U = 85,
            V = 86,
            W = 87,
            X = 88,
            Y = 89,
            Z = 90,
            LeftWindows = 91,
            RightWindows = 92,
            Applications = 93,
            Sleep = 95,
            NumPad0 = 96,
            NumPad1 = 97,
            NumPad2 = 98,
            NumPad3 = 99,
            NumPad4 = 100,
            NumPad5 = 101,
            NumPad6 = 102,
            NumPad7 = 103,
            NumPad8 = 104,
            NumPad9 = 105,
            Multiply = 106,
            Add = 107,
            Separator = 108,
            Subtract = 109,
            Decimal = 110,
            Divide = 111,
            F1 = 112,
            F2 = 113,
            F3 = 114,
            F4 = 115,
            F5 = 116,
            F6 = 117,
            F7 = 118,
            F8 = 119,
            F9 = 120,
            F10 = 121,
            F11 = 122,
            F12 = 123,
            F13 = 124,
            F14 = 125,
            F15 = 126,
            F16 = 127,
            F17 = 128,
            F18 = 129,
            F19 = 130,
            F20 = 131,
            F21 = 132,
            F22 = 133,
            F23 = 134,
            F24 = 135,
            BrowserBack = 166,
            BrowserForward = 167,
            BrowserRefresh = 168,
            BrowserStop = 169,
            BrowserSearch = 170,
            BrowserFavorites = 171,
            BrowserHome = 172,
            VolumeMute = 173,
            VolumeDown = 174,
            VolumeUp = 175,
            MediaNext = 176,
            MediaPrevious = 177,
            MediaStop = 178,
            MediaPlay = 179,
            LaunchMail = 180,
            LaunchMediaSelect = 181,
            LaunchApp1 = 182,
            LaunchApp2 = 183,
            Oem1 = 186,
            OemPlus = 187,
            OemComma = 188,
            OemMinus = 189,
            OemPeriod = 190,
            Oem2 = 191,
            Oem3 = 192,
            Oem4 = 219,
            Oem5 = 220,
            Oem6 = 221,
            Oem7 = 222,
            Oem8 = 223,
            Oem102 = 226,
            Process = 229,
            Packet = 231,
            Attention = 246,
            CrSel = 247,
            ExSel = 248,
            EraseEndOfFile = 249,
            Play = 250,
            Zoom = 251,
            NoName = 252,
            Pa1 = 253,
            OemClear = 254,
        } enumType;

        static std::vector<String> GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = { 
            _T("Backspace"), _T("Tab"), _T("Clear"), _T("Enter"), _T("Pause"), _T("Escape"), _T("Spacebar"), _T("PageUp"),
            _T("PageDown"), _T("End"), _T("Home"), _T("LeftArrow"), _T("UpArrow"), _T("RightArrow"), _T("DownArrow"), _T("Select"),
            _T("Print"), _T("Execute"), _T("PrintScreen"), _T("Insert"), _T("Delete"), _T("Help"),
            _T("D0"), _T("D1"), _T("D2"), _T("D3"), _T("D4"), _T("D5"), _T("D6"), _T("D7"), _T("D8"), _T("D9"),
            _T("A"), _T("B"), _T("C"), _T("D"), _T("E"), _T("F"), _T("G"), _T("H"), _T("I"), _T("J"), _T("K"), _T("L"), _T("M"), _T("N"),
            _T("O"), _T("P"), _T("Q"), _T("R"), _T("S"), _T("T"), _T("U"), _T("V"), _T("W"), _T("X"), _T("Y"), _T("Z"),
            _T("LeftWindows"), _T("RightWindows"), _T("Applications"), _T("Sleep"),
            _T("NumPad0"), _T("NumPad1"), _T("NumPad2"), _T("NumPad3"), _T("NumPad4"), _T("NumPad5"), _T("NumPad6"), _T("NumPad7"), _T("NumPad8"), _T("NumPad9"),
            _T("Multiply"), _T("Add"), _T("Separator"), _T("Subtract"), _T("Decimal"), _T("Divide"),
            _T("F1"), _T("F2"), _T("F3"), _T("F4"), _T("F5"), _T("F6"), _T("F7"), _T("F8"), _T("F9"), _T("F10"), _T("F11"), _T("F12"),
            _T("F13"), _T("F14"), _T("F15"), _T("F16"), _T("F17"), _T("F18"), _T("F19"), _T("F20"), _T("F21"), _T("F22"), _T("F23"), _T("F24"),
            _T("BrowserBack"), _T("BrowserForward"), _T("BrowserRefresh"), _T("BrowserStop"), _T("BrowserSearch"), _T("BrowserFavorites"),
            _T("BrowserHome"), _T("VolumeMute"), _T("VolumeDown"), _T("VolumeUp"), _T("MediaNext"), _T("MediaPrevious"), _T("MediaStop"),
            _T("MediaPlay"), _T("LaunchMail"), _T("LaunchMediaSelect"), _T("LaunchApp1"), _T("LaunchApp2"),
            _T("Oem1"), _T("OemPlus"), _T("OemComma"), _T("OemMinus"), _T("OemPeriod"), _T("Oem2"), _T("Oem3"), _T("Oem4"), _T("Oem5"), _T("Oem6"), _T("Oem7"), _T("Oem8"), _T("Oem102"),
            _T("Process"), _T("Packet"), _T("Attention"), _T("CrSel"), _T("ExSel"), _T("EraseEndOfFile"), _T("Play"), _T("Zoom"),
            _T("NoName"), _T("Pa1"), _T("OemClear")
        }; v = std::vector<String>(args, args + numItems); } return v; }

        static std::vector<enumType> GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {
            Backspace, Tab, Clear, Enter, Pause, Escape, Spacebar, PageUp,
            PageDown, End, Home, LeftArrow, UpArrow, RightArrow, DownArrow, Select,
            Print, Execute, PrintScreen, Insert, Delete, Help,
            D0, D1, D2, D3, D4, D5, D6, D7, D8, D9,
            A, B, C, D, E, F, G, H, I, J, K, L, M, N,
            O, P, Q, R, S, T, U, V, W, X, Y, Z,
            LeftWindows, RightWindows, Applications, Sleep,
            NumPad0, NumPad1, NumPad2, NumPad3, NumPad4, NumPad5, NumPad6, NumPad7, NumPad8, NumPad9,
            Multiply, Add, Separator, Subtract, Decimal, Divide,
            F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
            F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24,
            BrowserBack, BrowserForward, BrowserRefresh, BrowserStop, BrowserSearch, BrowserFavorites,
            BrowserHome, VolumeMute, VolumeDown, VolumeUp, MediaNext, MediaPrevious, MediaStop,
            MediaPlay, LaunchMail, LaunchMediaSelect, LaunchApp1, LaunchApp2,
            Oem1, OemPlus, OemComma, OemMinus, OemPeriod, Oem2, Oem3, Oem4, Oem5, Oem6, Oem7, Oem8, Oem102,
            Process, Packet, Attention, CrSel, ExSel, EraseEndOfFile, Play, Zoom,
            NoName, Pa1, OemClear }; v = std::vector<enumType>(args, args + numItems); } return v;
        }
        ENUM_IMPL(ConsoleKey, Backspace)
    };

    struct ConsoleKeyInfo
    {
        public: 
            ConsoleKeyInfo(Char keyChar, ConsoleKey key, bool shift, bool alt, bool control)
            {
                m_keyChar = keyChar;
                m_key = key;
                m_modifs = ((shift) ? ConsoleModifiers::Shift : 0)|
                            ((alt) ? ConsoleModifiers::Alt : 0) |
                            ((control) ? ConsoleModifiers::Control : 0);

            }
    
        //public: static bool operator !=(const ConsoleKeyInfo &a, const ConsoleKeyInfo &b)
        //{
        //}
        //public: static bool operator ==(const ConsoleKeyInfo &a, const ConsoleKeyInfo &b)
        //{
        //}
    
    public: ConsoleKey Key() const { return m_key; }     //property
    public: Char KeyChar() const { return m_keyChar; } //property
    public: ConsoleModifiers Modifiers() const { return m_modifs; } //property
        public: bool Equals(const ConsoleKeyInfo &obj)
        {
            return false;
        }
            //        public: override bool Equals(object value) {}
            //        public: override int GetHashCode();

        private: 
            Char m_keyChar;
            ConsoleKey m_key;
            ConsoleModifiers m_modifs;
    };

    class Console
    {
        private:

        class ConsoleState
        {
            HANDLE hstdin;
            HANDLE hstdout;
            CONSOLE_SCREEN_BUFFER_INFO csbi, old_csbi;

            public:
                ConsoleState()
                {
                    hstdin  = GetStdHandle( STD_INPUT_HANDLE  );
                    hstdout = GetStdHandle( STD_OUTPUT_HANDLE );
                    GetConsoleScreenBufferInfo( hstdout, &old_csbi );
                }
                ~ConsoleState()
                {
                    SetConsoleTextAttribute( hstdout,old_csbi.wAttributes);
                }

            ConsoleColor GetForeGroundColor()
            {
                GetConsoleScreenBufferInfo( hstdout, &csbi );
                return (ConsoleColor::enumType)(csbi.wAttributes & 0x0F);
            }
            ConsoleColor GetBackGroundColor()
            {
                GetConsoleScreenBufferInfo( hstdout, &csbi );
                return (ConsoleColor::enumType)(csbi.wAttributes >> 4);
            }
            void SetBackGroundColor(ConsoleColor color)
            {
                GetConsoleScreenBufferInfo( hstdout, &csbi );
                SetConsoleTextAttribute( hstdout, (csbi.wAttributes & 0x0F) | (((WORD)color) << 4));
            }
            void SetForeGroundColor(ConsoleColor color)
            {
                GetConsoleScreenBufferInfo( hstdout, &csbi );
                SetConsoleTextAttribute( hstdout, (csbi.wAttributes & 0xF0) | ((WORD)color));
            }


        };

        public:

            static ConsoleColor ForeGroundColor()
            {
                return GetConsoleState().GetForeGroundColor();
            }
            static ConsoleColor BackGroundColor()
            {
                return GetConsoleState().GetBackGroundColor();
            }

            static void ForeGroundColor(ConsoleColor color)
            {
                GetConsoleState().SetForeGroundColor(color);
            }

            static void BackGroundColor(ConsoleColor color)
            {
                GetConsoleState().SetBackGroundColor(color);
            }

            static void Write(const Char &c) { _fputtc(c,stdout); }
            static void Write(const String &s) { _fputts(s.c_str(),stdout); }
            TEMP_ARG_1 static void Write(const String &text, TCREF_ARG_1) { Write(String::Format(text,ARG_1)); }
            TEMP_ARG_2 static void Write(const String &text, TCREF_ARG_2) { Write(String::Format(text,ARG_2)); }
            TEMP_ARG_3 static void Write(const String &text, TCREF_ARG_3) { Write(String::Format(text,ARG_3)); }
            TEMP_ARG_4 static void Write(const String &text, TCREF_ARG_4) { Write(String::Format(text,ARG_4)); }
            TEMP_ARG_5 static void Write(const String &text, TCREF_ARG_5) { Write(String::Format(text,ARG_5)); }
            TEMP_ARG_6 static void Write(const String &text, TCREF_ARG_6) { Write(String::Format(text,ARG_6)); }
            TEMP_ARG_7 static void Write(const String &text, TCREF_ARG_7) { Write(String::Format(text,ARG_7)); }
            TEMP_ARG_8 static void Write(const String &text, TCREF_ARG_8) { Write(String::Format(text,ARG_8)); }
            TEMP_ARG_9 static void Write(const String &text, TCREF_ARG_9) { Write(String::Format(text,ARG_9)); }
            TEMP_ARG_10 static void Write(const String &text, TCREF_ARG_10) { Write(String::Format(text,ARG_10)); }

            static void WriteLine(const Char &c) { _fputtc(c,stdout); _fputtc(_T('\n'),stdout);}
            static void WriteLine() { _fputts(_T("\n"),stdout); }
            static void WriteLine(const String &s) { _fputts(s.c_str(),stdout); _fputtc(_T('\n'),stdout);  }
            TEMP_ARG_1 static void WriteLine(const String &text, TCREF_ARG_1) { WriteLine(String::Format(text,ARG_1)); }
            TEMP_ARG_2 static void WriteLine(const String &text, TCREF_ARG_2) { WriteLine(String::Format(text,ARG_2)); }
            TEMP_ARG_3 static void WriteLine(const String &text, TCREF_ARG_3) { WriteLine(String::Format(text,ARG_3)); }
            TEMP_ARG_4 static void WriteLine(const String &text, TCREF_ARG_4) { WriteLine(String::Format(text,ARG_4)); }
            TEMP_ARG_5 static void WriteLine(const String &text, TCREF_ARG_5) { WriteLine(String::Format(text,ARG_5)); }
            TEMP_ARG_6 static void WriteLine(const String &text, TCREF_ARG_6) { WriteLine(String::Format(text,ARG_6)); }
            TEMP_ARG_7 static void WriteLine(const String &text, TCREF_ARG_7) { WriteLine(String::Format(text,ARG_7)); }
            TEMP_ARG_8 static void WriteLine(const String &text, TCREF_ARG_8) { WriteLine(String::Format(text,ARG_8)); }
            TEMP_ARG_9 static void WriteLine(const String &text, TCREF_ARG_9) { WriteLine(String::Format(text,ARG_9)); }
            TEMP_ARG_10 static void WriteLine(const String &text, TCREF_ARG_10) { WriteLine(String::Format(text,ARG_10)); }
            static String ReadLine()
            {
                Char A[2000];
                _fgetts( A, sizeof(A), stdin);
                String line = A;
                line = line.TrimEnd(_T("\r\n"));
                return line;
            }

            //bool KeyAvailable() const   // property
            //{
            //    int i = _kbhit();
            //    return i!=0;
            //}

            //static ConsoleKeyInfo ReadKey()
            //{
            //    return ReadKey(false);
            //}
            //static ConsoleKeyInfo ReadKey(bool intercept)
            //{
            //    int i = _getch();
            //    char c = (char)i;
            //    return ConsoleKeyInfo();
            //}
    private:
        static ConsoleState& GetConsoleState()
        {
            static ConsoleState state;
            return state;
        }
    };
}

