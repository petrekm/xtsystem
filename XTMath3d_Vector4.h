
namespace XTMath3d
{

    class Vector4
    {
    public:
        double x, y, z, w;

        Vector4() : x(), y(), z(), w() {}

        Vector4(const Vector4& other)
        {
            x = other.x;
            y = other.y;
            z = other.z;
            w = other.w;
        }

        Vector4(double ax, double ay, double az, double aw)
        {
            x = ax;
            y = ay;
            z = az;
            w = aw;
        }

        //static Vector4 xAxis = new Vector4(1, 0, 0, 1);
        //static Vector4 yAxis = new Vector4(0, 1, 0, 1);
        //static Vector4 zAxis = new Vector4(0, 0, 1, 1);
        //static Vector4 origin = new Vector4(0, 0, 0, 1);

        static Vector4 Create(double ax, double ay, double az, double aw)
        {
            return Vector4(ax, ay, az, aw);
        }
        static Vector4 Create(double ax, double ay, double az)
        {
            return Vector4(ax, ay, az, 1);
        }
        static double Norm(const Vector4 &a)
        {
            return a.Norm();
        }

        static double Norm2(const Vector4 &a)
        {
            return a.Norm2();
        }

        double Norm() const
        {
            return (double)sqrt((x * x + y * y + z * z)/(w*w));
        }
        double Norm2() const
        {
            return (x * x + y * y + z * z)/(w*w);
        }
        static double Distance(const Vector4 &a, const Vector4 &b)
        {
            return (a - b).Norm();
        }
        static double Distance2(const Vector4 &a, const Vector4 &b)
        {
            return (a - b).Norm2();
        }

        static double Dot(const Vector4 &a, const Vector4 &b)
        {
            return a.Dot(b);
        }

        double Dot(const Vector4 &other) const
        {
            return (x * other.x + y * other.y + z * other.z)/(w*other.w);
        }

        static Vector4 Cross(const Vector4 &a, const Vector4 &b)
        {
            return a.Cross(b);
        }

        Vector4 Cross(const Vector4 &other) const
        {
            return Vector4(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x, w * other.w);
        }

        //public double this[int i]
        //{
        //    get
        //    {
        //        switch (i)
        //        {
        //            case 0: return x; break;
        //            case 1: return y; break;
        //            case 2: return z; break;
        //            case 3: return w; break;
        //        }
        //        return 0;
        //    }
        //    set
        //    {
        //        switch (i)
        //        {
        //            case 0: x = value; break;
        //            case 1: y = value; break;
        //            case 2: z = value; break;
        //            case 3: w = value; break;
        //        }
        //    }
        //}

        void Set(double ax, double ay, double az)
        {
            x = ax;
            y = ay;
            z = az;
            w = 1;
        }

        void Set(double ax, double ay, double az, double aw)
        {
            x = ax;
            y = ay;
            z = az;
            w = aw;
        }

        friend Vector4 operator +(const Vector4 &a, const Vector4 &b)
        {
            return Vector4(b.w * a.x + a.w * b.x, b.w * a.y + a.w * b.y, b.w * a.z + a.w * b.z, a.w * b.w);
        }
        friend Vector4 operator -(const Vector4 &a, const Vector4 &b)
        {
            return Vector4(b.w * a.x - a.w * b.x, b.w * a.y - a.w * b.y, b.w * a.z - a.w * b.z, a.w * b.w);
        }

        friend Vector4 operator *(const Vector4 &a, double b)
        {
            return Vector4(a.x * b, a.y * b, a.z * b, a.w);
        }

        friend Vector4 operator *(double b, const Vector4 &a)
        {
            return Vector4(a.x * b, a.y * b, a.z * b, a.w);
        }

        friend Vector4 operator /(const Vector4 &a, double b)
        {
            return Vector4(a.x / b, a.y / b, a.z / b, a.w);
        }

        friend Vector4 operator -(const Vector4 &other)
        {

            return Vector4(-other.x, -other.y, -other.z, other.w);
        }

    };

}
