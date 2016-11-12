
namespace XTMath3d
{
    class Vector3
    {
        public:

        double x, y, z;
        Vector3() : x(), y(), z() {}

        Vector3(const Vector3& other)
        {
            x = other.x;
            y = other.y;
            z = other.z;
        }

        Vector3(double ax, double ay, double az)
        {
            x = ax;
            y = ay;
            z = az;
        }
        //static Vector3 xAxis = new Vector3(1, 0, 0);
        //static Vector3 yAxis = new Vector3(0, 1, 0);
        //static Vector3 zAxis = new Vector3(0, 0, 1);
        //static Vector3 origin = new Vector3(0, 0, 0);

        static Vector3 Create(double ax, double ay, double az)
        {
            return Vector3(ax, ay, az);
        }
        static double Norm(const Vector3 &a)
        {
            return a.Norm();
        }

        static double Norm2(const Vector3 &a)
        {
            return a.Norm2();
        }

        double Norm() const
        {
            return (double)sqrt(x * x + y * y + z * z);
        }
        double Norm2() const
        {
            return x * x + y * y + z * z;
        }
        static double Distance(Vector3& a, Vector3& b)
        {
            double dx = a.x - b.x;
            double dy = a.y - b.y;
            double dz = a.z - b.z;
            return (double)sqrt(dx * dx + dy * dy + dz * dz);
        }
        static double Distance2(Vector3& a, Vector3& b)
        {
            double dx = a.x - b.x;
            double dy = a.y - b.y;
            double dz = a.z - b.z;
            return dx * dx + dy * dy + dz * dz;
        }

        static double Dot(const Vector3& a, const Vector3& b)
        {
            return a.Dot(b);
        }

        double Dot(const Vector3& other) const
        {
            return x * other.x + y * other.y + z * other.z;
        }

        static Vector3 Cross(const Vector3& a, const Vector3& b)
        {
            return a.Cross(b);
        }

        Vector3 Cross(const Vector3& other) const
        {
            return Vector3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
        }

        //double this[int i]
        //{
        //    get
        //    {
        //        switch (i)
        //        {
        //            case 0: return x; break;
        //            case 1: return y; break;
        //            case 2: return z; break;
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
        //        }
        //    }
        //}

        void Set(double ax,double ay, double az)
        {
            x = ax;
            y = ay;
            z = az;
        }

        friend Vector3 operator+(const Vector3& a,const Vector3& b)
        {
            return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
        }
        friend Vector3 operator -(const Vector3 &a, const Vector3 &b)
        {
            return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
        }

        friend Vector3 operator *(const Vector3 &a, double b)
        {
            return Vector3(a.x * b, a.y * b, a.z * b);
        }

        friend Vector3 operator *(double b, const Vector3 &a)
        {
            return Vector3(a.x * b, a.y * b, a.z * b);
        }

        friend Vector3 operator /(const Vector3 &a, double b)
        {
            return Vector3(a.x / b, a.y / b, a.z / b);
        }

        friend Vector3 operator -(const Vector3 &other)
        {
            return Vector3(-other.x, -other.y, -other.z);
        }

    };

}
