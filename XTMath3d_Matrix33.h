
namespace XTMath3d
{
    class Matrix33
    {
    public:
        Matrix33(): mV00(),mV01(),mV02(),
                     mV10(),mV11(),mV12(),
                     mV20(),mV21(),mV22() {}

        Matrix33(const Matrix33& other)
        {
            mV00 = other.mV00; mV01 = other.mV01; mV02 = other.mV02;
            mV10 = other.mV10; mV11 = other.mV11; mV12 = other.mV12;
            mV20 = other.mV20; mV21 = other.mV21; mV22 = other.mV22;
        }

        Matrix33(double init)
        {
            mV00 = init;
            mV01 = init;
            mV02 = init;
            mV10 = init;
            mV11 = init;
            mV12 = init;
            mV20 = init;
            mV21 = init;
            mV22 = init;
        }

        Matrix33(double a00, double a01, double a02, double a10, double a11, double a12, double a20, double a21, double a22)
        {
            mV00 = a00;
            mV01 = a01;
            mV02 = a02;
            mV10 = a10;
            mV11 = a11;
            mV12 = a12;
            mV20 = a20;
            mV21 = a21;
            mV22 = a22;

        }

        void GetValues(double *a00, double *a01, double *a02, double *a10, double *a11, double *a12, double *a20, double *a21, double *a22)
        {
            *a00 = mV00;
            *a01 = mV01;
            *a02 = mV02;
            *a10 = mV10;
            *a11 = mV11;
            *a12 = mV12;
            *a20 = mV20;
            *a21 = mV21;
            *a22 = mV22;

        }
        //public double this[int i, int j]
        //{
        //    get
        //    {
        //        return Get(i, j);
        //    }
        //    set
        //    {
        //        Set(i, j, value);
        //    }
        //}

        //Based on http://en.wikipedia.org/wiki/Rotation_matrix
        static Matrix33 CreateRotationalX(double alpha)
        {
            double s = (double)sin(alpha);
            double c = (double)cos(alpha);
            return Matrix33(
                1, 0, 0,
                0, c, -s,
                0, s, c);
        }
        static Matrix33 CreateRotationalY(double alpha)
        {
            double s = (double)sin(alpha);
            double c = (double)cos(alpha);
            return Matrix33(
                c, 0, s,
                0, 1, 0,
                -s, 0, c);
        }

        static Matrix33 CreateRotationalZ(double alpha)
        {
            double s = (double)sin(alpha);
            double c = (double)cos(alpha);
            return Matrix33(
                c, -s, 0,
                s, c, 0,
                0, 0, 1);
        }

        //static Matrix33 identity = new Matrix33(1, 0, 0, 0, 1, 0, 0, 0, 1);
        Vector3 GetColumn(int j)
        {
            switch (j)
            {
                case 0: return Vector3(mV00, mV10, mV20);
                case 1: return Vector3(mV01, mV11, mV21);
                case 2: return Vector3(mV02, mV12, mV22);
            }
            return Vector3();
        }
        Vector3 GetRow(int i)
        {
            switch (i)
            {
                case 0: return Vector3(mV00, mV01, mV02);
                case 1: return Vector3(mV10, mV11, mV12);
                case 2: return Vector3(mV20, mV21, mV22);
            }
            return Vector3();
        }

        double Get(int i, int j)
        {

            switch (i)
            {
                case 0:
                    switch (j)
                    {
                        case 0: return mV00;
                        case 1: return mV01;
                        case 2: return mV02;
                    }
                    break;
                case 1: switch (j)
                    {
                        case 0: return mV10;
                        case 1: return mV11;
                        case 2: return mV12;
                    }
                    break;
                case 2: switch (j)
                    {
                        case 0: return mV20;
                        case 1: return mV21;
                        case 2: return mV22;
                    }
                    break;
            }
            return 0;
        }
        void Set(int i, int j, double value)
        {
            switch (i)
            {
                case 0: switch (j)
                    {
                        case 0: mV00 = value; return;
                        case 1: mV01 = value; return;
                        case 2: mV02 = value; return;
                    }
                    break;
                case 1: switch (j)
                    {
                        case 0: mV10 = value; return;
                        case 1: mV11 = value; return;
                        case 2: mV12 = value; return;
                    }
                    break;
                case 2: switch (j)
                    {
                        case 0: mV20 = value; return;
                        case 1: mV21 = value; return;
                        case 2: mV22 = value; return;
                    }
                    break;
            }
        }
        void SetColumn(int i, const Vector3 &c)
        {
            Set(0, i, c.x);
            Set(1, i, c.y);
            Set(2, i, c.z);
        }
        void SetRow(int i, const Vector3 &c)
        {
            Set(i, 0, c.x);
            Set(i, 1, c.y);
            Set(i, 2, c.z);
        }

        void SetColumns(const Vector3 &c0, const Vector3 &c1, const Vector3 &c2)
        {
            SetColumn(0, c0);
            SetColumn(1, c1);
            SetColumn(2, c2);
        }
        void SetRows(const Vector3 &r0, const Vector3 &r1, const Vector3 &r2)
        {
            SetRow(0, r0);
            SetRow(1, r1);
            SetRow(2, r2);
        }

        friend Matrix33 operator +(const Matrix33 &a, const Matrix33 &b)
        {
            return Matrix33(a.mV00 + b.mV00, a.mV01 + b.mV01, a.mV02 + b.mV02,
                                 a.mV10 + b.mV10, a.mV11 + b.mV11, a.mV12 + b.mV12,
                                 a.mV20 + b.mV20, a.mV21 + b.mV21, a.mV22 + b.mV22);
        }

        friend Matrix33 operator -(const Matrix33 &a, const Matrix33 &b)
        {
            return Matrix33(a.mV00 - b.mV00, a.mV01 - b.mV01, a.mV02 - b.mV02,
                                 a.mV10 - b.mV10, a.mV11 - b.mV11, a.mV12 - b.mV12,
                                 a.mV20 - b.mV20, a.mV21 - b.mV21, a.mV22 - b.mV22);
        }

        friend Matrix33 operator *(const Matrix33 &a, double b)
        {
            return Matrix33(a.mV00 * b, a.mV01 * b, a.mV02 * b,
                                 a.mV10 * b, a.mV11 * b, a.mV12 * b,
                                 a.mV20 * b, a.mV21 * b, a.mV22 * b);
        }
        friend Matrix33 operator /(const Matrix33 &a, double b)
        {
            return Matrix33(a.mV00 / b, a.mV01 / b, a.mV02 / b,
                                 a.mV10 / b, a.mV11 / b, a.mV12 / b,
                                 a.mV20 / b, a.mV21 / b, a.mV22 / b);
        }

        friend Matrix33 Abs(const Matrix33 &a)
        {
            return Matrix33(fabs(a.mV00), fabs(a.mV01), fabs(a.mV02),
                                 fabs(a.mV10), fabs(a.mV11), fabs(a.mV12),
                                 fabs(a.mV20), fabs(a.mV21), fabs(a.mV22));
        }

        friend Matrix33 operator *(double b, const Matrix33 &a)
        {
            return Matrix33(a.mV00 * b, a.mV01 * b, a.mV02 * b,
                                 a.mV10 * b, a.mV11 * b, a.mV12 * b,
                                 a.mV20 * b, a.mV21 * b, a.mV22 * b);
        }
        friend Matrix33 operator *(const Matrix33 &a, const Matrix33 &b)
        {

            return Matrix33(a.mV00 * b.mV00 + a.mV01 * b.mV10 + a.mV02 * b.mV20,
                                 a.mV00 * b.mV01 + a.mV01 * b.mV11 + a.mV02 * b.mV21,
                                 a.mV00 * b.mV02 + a.mV01 * b.mV12 + a.mV02 * b.mV22,

                                 a.mV10 * b.mV00 + a.mV11 * b.mV10 + a.mV12 * b.mV20,
                                 a.mV10 * b.mV01 + a.mV11 * b.mV11 + a.mV12 * b.mV21,
                                 a.mV10 * b.mV02 + a.mV11 * b.mV12 + a.mV12 * b.mV22,

                                 a.mV20 * b.mV00 + a.mV21 * b.mV10 + a.mV22 * b.mV20,
                                 a.mV20 * b.mV01 + a.mV21 * b.mV11 + a.mV22 * b.mV21,
                                 a.mV20 * b.mV02 + a.mV21 * b.mV12 + a.mV22 * b.mV22);
        }

        friend Matrix33 Transpose(const Matrix33 &M)
        {
            return Matrix33(M.mV00, M.mV10, M.mV20, 
                                 M.mV01, M.mV11, M.mV21, 
                                 M.mV02, M.mV12, M.mV22);
        }

        friend Vector3 operator *(const Matrix33 &M, const Vector3 &p)
        {
            Vector3 result;

            result.x = M.mV00 * p.x + M.mV01 * p.y + M.mV02 * p.z;
            result.y = M.mV10 * p.x + M.mV11 * p.y + M.mV12 * p.z;
            result.z = M.mV20 * p.x + M.mV21 * p.y + M.mV22 * p.z;

            return result;

        }
        friend Vector3 operator *(const Vector3 &p,const Matrix33 &M)
        {
            Vector3 result;

            result.x = M.mV00 * p.x + M.mV10 * p.y + M.mV20 * p.z;
            result.y = M.mV01 * p.x + M.mV11 * p.y + M.mV21 * p.z;
            result.z = M.mV02 * p.x + M.mV12 * p.y + M.mV22 * p.z;

            return result;

        }

        void MakeIdentity()
        {
            mV00 = 1.0;
            mV01 = 0.0;
            mV02 = 0.0;
            mV10 = 0.0;
            mV11 = 1.0;
            mV12 = 0.0;
            mV20 = 0.0;
            mV21 = 0.0;
            mV22 = 1.0;

        }

        private:
            double mV00, mV01, mV02;
            double mV10, mV11, mV12;
            double mV20, mV21, mV22;
    };

}
