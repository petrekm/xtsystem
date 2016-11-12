
namespace XTSystem
{

    enum_4(RangeType, ClosedClosed,OpenClosed,ClosedOpen,OpenOpen)

struct Math
{
    static const double Pi() { return 3.14159265358979323846;}
    static const double E() { return 2.718281828459045;}
    static double Rad2Deg(double x) { return 180.0 * x / Pi(); }
    static double Deg2Rad(double x) { return Pi() * x / 180.0; }
    static double RadiansToDegrees(double x) { return Rad2Deg(x); }
    static double DegreesToRadians(double x) { return Deg2Rad(x); }
    static double Sqr(double x) { return x * x; }
    static double RandomUniform01() { return ((double) rand ()) / (((double) RAND_MAX) +1.0); }
    static double RandomUniformAB(double a, double b) { return (b - a) * RandomUniform01() + a; }
    static double NormalizeAngleToRange0_360(double xDEG)
    {
        double res = fmod(xDEG, 360.0);
        if (res<0) res += 360.0;
        return res;
    }

    static double NormalizeAngleToRange_180plus180(double xDEG)
    {
        double d = NormalizeAngleToRange0_360(xDEG);
        if (d>180) d -=360;
        return d;
    }
    static double NormalizeAngleToRange0_2Pi(double xRAD)
    {
        return Deg2Rad(NormalizeAngleToRange0_360(Rad2Deg(xRAD)));
    }
    static double NormalizeAngleToRange_PiPi(double xRAD)
    {
        return Deg2Rad(NormalizeAngleToRange_180plus180(Rad2Deg(xRAD)));
    }
    static double AngleDiffIn_180plus180(double xDEG, double yDEG)
    {
        double d = NormalizeAngleToRange0_360(xDEG - yDEG);
        if (d > 180) d = d - 360;
        return d;
    }
    static double AngleDiffIn_PIplusPI(double xRAD, double yRAD)
    {
        return Deg2Rad(AngleDiffIn_180plus180(Rad2Deg(xRAD), Rad2Deg(yRAD)));
    }
    template<class T>static T Abs(T x) { return (x>0)?x:-x; }
    static double Acos(double x) { return acos(x); }
    static double Asin(double x) { return asin(x); }
    static double Atan(double x) { return atan(x); }
    static double Atan2(double y,double x) { return atan2(y,x); }
    static double Ceiling(double x) { return ceil(x); }
    static double Cos(double x) { return cos(x); }
    static double Cosh(double x) { return cosh(x); }
    static double Exp(double x) { return exp(x); }
    static double FMod(double x,double y) { return fmod(x,y); }
    static double Floor(double x) { return floor(x); }
    static double Log(double x) { return log(x); }
    static double Log(double a, double base) { return log(a)/log(base); }
    static double Log10(double x) { return log10(x); }
    template<class T>static T Max(T a, T b) { return (a>b)?a:b; }
    template<class T>static T Min(T a, T b) { return (a<b)?a:b; }
    static double Pow(double x, double y) { return pow(x,y); }
    template<class T>static int Sign(T x) { return (x>0)?1:(x<0)?-1:0; }
    static double Sin(double x) { return sin(x); }
    static double Sinh(double x) { return sinh(x); }
    static double Sqrt(double x) { return sqrt(x); }
    static double Tan(double x) { return tan(x); }
    static double Tanh(double x) { return tanh(x); }
    template<class T>static T Round(T x) { return (T)((long)(x+0.5)); }
    template<class T>static T Truncate(T x) { return (T)((long)(x)); }

    template<class T>
    static bool ValueInRange(T min, T max, T value, RangeType type = RangeType::ClosedClosed)
    {
        if (min > max)
        {
            T a = min;
            min = max;
            max = a;
        }

        if (type == RangeType::ClosedClosed) return value >= min && value <= max;
        if (type == RangeType::OpenClosed) return value > min && value <= max;
        if (type == RangeType::ClosedOpen) return value >= min && value < max;
        if (type == RangeType::OpenOpen) return value > min && value < max;
        return false;
    }

    static bool AngleInRange(double minDEG, double maxDEG, double valueDEG)
    {
        if (minDEG > maxDEG) 
        {
            throw Exception(L"maxDEG must be greater than minDEG");
        }

        valueDEG = Math::NormalizeAngleToRange0_360(valueDEG);
        minDEG = Math::NormalizeAngleToRange0_360(minDEG);
        maxDEG = Math::NormalizeAngleToRange0_360(maxDEG);
        if (minDEG <= maxDEG)
        {
            return ValueInRange(minDEG, maxDEG, valueDEG);
        }
        return !ValueInRange(maxDEG, minDEG, valueDEG,RangeType::OpenOpen);
    }

    static bool AngleInRangeEx(double baseDEG, double tolLeftDEG, double tolRightDEG, double valueDEG)
    {
        if (tolLeftDEG < 0 || tolRightDEG < 0) throw Exception(L"tolLeftDEG and tolRightDEG must be positive or 0");
        if (tolRightDEG + tolLeftDEG >=360) return true;    //we covered whole circle
        return AngleInRange(baseDEG - tolLeftDEG, baseDEG + tolRightDEG, valueDEG);
    }
};

}

