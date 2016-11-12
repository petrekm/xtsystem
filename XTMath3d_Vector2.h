
namespace XTMath3d
{
    class Vector2
    {
        public:

        double x, y;
        Vector2() : x(), y() {}

        Vector2(const Vector2& other)
        {
            x = other.x;
            y = other.y;
        }

        Vector2(double ax, double ay)
        {
            x = ax;
            y = ay;
        }
        //static Vector2 xAxis = new Vector2(1, 0);
        //static Vector2 yAxis = new Vector2(0, 1);
        //static Vector2 origin = new Vector2(0, 0);

        static Vector2 Create(double ax, double ay)
        {
            return Vector2(ax, ay);
        }
        static double Norm(const Vector2 &a)
        {
            return a.Norm();
        }

        static double Norm2(const Vector2 &a)
        {
            return a.Norm2();
        }

        double Norm() const
        {
            return (double)sqrt(x * x + y * y);
        }
        double Norm2() const
        {
            return x * x + y * y;
        }
        static double Distance(Vector2& a, Vector2& b)
        {
            double dx = a.x - b.x;
            double dy = a.y - b.y;
            return (double)sqrt(dx * dx + dy * dy);
        }
        static double Distance2(Vector2& a, Vector2& b)
        {
            double dx = a.x - b.x;
            double dy = a.y - b.y;
            return dx * dx + dy * dy;
        }

        static double Dot(const Vector2& a, const Vector2& b)
        {
            return a.Dot(b);
        }

        double Dot(const Vector2& other) const
        {
            return x * other.x + y * other.y;
        }

        static Vector2 RotateCounterClockwise(const Vector2& a, double angleRAD)
        {
            double s = sin(angleRAD);
            double c = cos(angleRAD);
            return Vector2(c * a.x - s * a.y, s * a.x + c * a.y);
        }

        static Vector2 RotateClockwise(const Vector2& a, double angleRAD)
        {
            return RotateCounterClockwise(a, -angleRAD);
        }
/*
        static Vector3 Cross(const Vector2& a, const Vector2& b)
        {
            return a.Cross(b);
        }

        Vector3 Cross(const Vector2& other) const
        {
            return Vector3(0, 0, x * other.y - y * other.x);
        }
*/
        //double this[int i]
        //{
        //    get
        //    {
        //        switch (i)
        //        {
        //            case 0: return x; break;
        //            case 1: return y; break;
        //        }
        //        return 0;
        //    }
        //    set
        //    {
        //        switch (i)
        //        {
        //            case 0: x = value; break;
        //            case 1: y = value; break;
        //        }
        //    }
        //}

        void Set(double ax,double ay)
        {
            x = ax;
            y = ay;
        }

        friend Vector2 operator+(const Vector2& a,const Vector2& b)
        {
            return Vector2(a.x + b.x, a.y + b.y);
        }
        friend Vector2 operator -(const Vector2 &a, const Vector2 &b)
        {
            return Vector2(a.x - b.x, a.y - b.y);
        }

        friend Vector2 operator *(const Vector2 &a, double b)
        {
            return Vector2(a.x * b, a.y * b);
        }

        friend Vector2 operator *(double b, const Vector2 &a)
        {
            return Vector2(a.x * b, a.y * b);
        }

        friend Vector2 operator /(const Vector2 &a, double b)
        {
            return Vector2(a.x / b, a.y / b);
        }

        friend Vector2 operator -(const Vector2 &other)
        {
            return Vector2(-other.x, -other.y);
        }

    };

}
