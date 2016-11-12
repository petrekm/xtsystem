
namespace XTMath3d
{
    class Matrix44
    {
        public:

        Matrix44(): mV00(),mV01(),mV02(),mV03(),
                     mV10(),mV11(),mV12(),mV13(),
                     mV20(),mV21(),mV22(),mV23(),
                     mV30(),mV31(),mV32(),mV33()
        {}

        Matrix44(const Matrix44& other)
        {
            mV00 = other.mV00; mV01 = other.mV01; mV02 = other.mV02; mV03 = other.mV03;
            mV10 = other.mV10; mV11 = other.mV11; mV12 = other.mV12; mV13 = other.mV13;
            mV20 = other.mV20; mV21 = other.mV21; mV22 = other.mV22; mV23 = other.mV23;
            mV30 = other.mV30; mV31 = other.mV31; mV32 = other.mV32; mV33 = other.mV33;
        }

        Matrix44(double init)
        {
            mV00 = init;
            mV01 = init;
            mV02 = init;
            mV03 = init;
            mV10 = init;
            mV11 = init;
            mV12 = init;
            mV13 = init;
            mV20 = init;
            mV21 = init;
            mV22 = init;
            mV23 = init;
            mV30 = init;
            mV31 = init;
            mV32 = init;
            mV33 = init;


        }
        Matrix44(double a00, double a01, double a02, double a03,
                        double a10, double a11, double a12, double a13, 
                        double a20, double a21, double a22, double a23, 
                        double a30, double a31, double a32, double a33)
        {
            mV00 = a00;
            mV01 = a01;
            mV02 = a02;
            mV03 = a03;
            mV10 = a10;
            mV11 = a11;
            mV12 = a12;
            mV13 = a13;
            mV20 = a20;
            mV21 = a21;
            mV22 = a22;
            mV23 = a23;
            mV30 = a30;
            mV31 = a31;
            mV32 = a32;
            mV33 = a33;

        }

        void GetValues( double *a00,  double *a01,  double *a02,  double *a03,
                             double *a10,  double *a11,  double *a12,  double *a13,
                             double *a20,  double *a21,  double *a22,  double *a23,
                             double *a30,  double *a31,  double *a32,  double *a33)
        {
            *a00 = mV00;
            *a01 = mV01;
            *a02 = mV02;
            *a03 = mV03;

            *a10 = mV10;
            *a11 = mV11;
            *a12 = mV12;
            *a13 = mV13;

            *a20 = mV20;
            *a21 = mV21;
            *a22 = mV22;
            *a23 = mV23;

            *a30 = mV30;
            *a31 = mV31;
            *a32 = mV32;
            *a33 = mV33;

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
        static Matrix44 CreateRotationalX(double alpha)
        {
            double s = (double)sin(alpha);
            double c = (double)cos(alpha);
            return Matrix44(
                1, 0,  0, 0,
                0, c, -s, 0,
                0, s,  c, 0,
                0, 0,  0, 1);
        }
        static Matrix44 CreateRotationalY(double alpha)
        {
            double s = (double)sin(alpha);
            double c = (double)cos(alpha);
            return Matrix44(
                 c, 0, s, 0,
                 0, 1, 0, 0,
                -s, 0, c, 0,
                 0, 0, 0, 1);
        }

        static Matrix44 CreateRotationalZ(double alpha)
        {
            double s = (double)sin(alpha);
            double c = (double)cos(alpha);
            return Matrix44(
                c, -s, 0, 0,
                s,  c, 0, 0,
                0,  0, 1, 0,
                0,  0, 0, 1);
        }

        static Matrix44 CreateTranslation(double tx, double ty, double tz)
        {
            return Matrix44(
                1, 0, 0, tx,
                0, 1, 0, ty,
                0, 0, 1, tz,
                0, 0, 0, 1);
        }

        //public static Matrix44 identity = new Matrix44(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
        Vector4 GetColumn(int j)
        {
            switch (j)
            {
                case 0: return Vector4(mV00, mV10, mV20, mV30);
                case 1: return Vector4(mV01, mV11, mV21, mV31);
                case 2: return Vector4(mV02, mV12, mV22, mV32);
                case 3: return Vector4(mV03, mV13, mV23, mV33);
            }
            return Vector4();
        }
        Vector4 GetRow(int i)
        {
            switch (i)
            {
                case 0: return Vector4(mV00, mV01, mV02, mV03);
                case 1: return Vector4(mV10, mV11, mV12, mV13);
                case 2: return Vector4(mV20, mV21, mV22, mV23);
                case 3: return Vector4(mV30, mV31, mV32, mV33);
            }
            return Vector4();
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
                        case 3: return mV03;
                    }
                    break;
                case 1: switch (j)
                    {
                        case 0: return mV10;
                        case 1: return mV11;
                        case 2: return mV12;
                        case 3: return mV13;
                    }
                    break;
                case 2: switch (j)
                    {
                        case 0: return mV20;
                        case 1: return mV21;
                        case 2: return mV22;
                        case 3: return mV23;
                    }
                    break;
                case 3: switch (j)
                    {
                        case 0: return mV30;
                        case 1: return mV31;
                        case 2: return mV32;
                        case 3: return mV33;
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
                        case 3: mV03 = value; return;
                    }
                    break;
                case 1: switch (j)
                    {
                        case 0: mV10 = value; return;
                        case 1: mV11 = value; return;
                        case 2: mV12 = value; return;
                        case 3: mV13 = value; return;
                    }
                    break;
                case 2: switch (j)
                    {
                        case 0: mV20 = value; return;
                        case 1: mV21 = value; return;
                        case 2: mV22 = value; return;
                        case 3: mV23 = value; return;
                    }
                    break;
            }
        }
        void SetColumn(int i, const Vector4 &c)
        {
            Set(0, i, c.x);
            Set(1, i, c.y);
            Set(2, i, c.z);
            Set(3, i, c.w);
        }
        void SetRow(int i, const Vector4 &c)
        {
            Set(i, 0, c.x);
            Set(i, 1, c.y);
            Set(i, 2, c.z);
            Set(i, 3, c.w);
        }

        void SetColumns(const Vector4 &c0, const Vector4 &c1, const Vector4 &c2, const Vector4 &c3)
        {
            SetColumn(0, c0);
            SetColumn(1, c1);
            SetColumn(2, c2);
            SetColumn(3, c3);
        }
        void SetRows(const Vector4 &r0, const Vector4 &r1, const Vector4 &r2, const Vector4 &r3)
        {
            SetRow(0, r0);
            SetRow(1, r1);
            SetRow(2, r2);
            SetRow(3, r3);
        }

        friend Matrix44 operator +(const Matrix44 &a, const Matrix44 &b)
        {
            return Matrix44(a.mV00 + b.mV00, a.mV01 + b.mV01, a.mV02 + b.mV02, a.mV03 + b.mV03,
                                 a.mV10 + b.mV10, a.mV11 + b.mV11, a.mV12 + b.mV12, a.mV13 + b.mV13,
                                 a.mV20 + b.mV20, a.mV21 + b.mV21, a.mV22 + b.mV22, a.mV23 + b.mV23,
                                 a.mV30 + b.mV30, a.mV31 + b.mV31, a.mV32 + b.mV32, a.mV33 + b.mV33);
        }

        friend Matrix44 operator -(const Matrix44 &a, const Matrix44 &b)
        {
            return Matrix44(a.mV00 - b.mV00, a.mV01 - b.mV01, a.mV02 - b.mV02, a.mV03 - b.mV03,
                                 a.mV10 - b.mV10, a.mV11 - b.mV11, a.mV12 - b.mV12, a.mV13 - b.mV13,
                                 a.mV20 - b.mV20, a.mV21 - b.mV21, a.mV22 - b.mV22, a.mV23 - b.mV23,
                                 a.mV30 - b.mV30, a.mV31 - b.mV31, a.mV32 - b.mV32, a.mV33 - b.mV33);
        }

        friend Matrix44 operator *(const Matrix44 &a, double b)
        {
            return Matrix44(a.mV00 * b, a.mV01 * b, a.mV02 * b, a.mV03 * b,
                                 a.mV10 * b, a.mV11 * b, a.mV12 * b, a.mV13 * b, 
                                 a.mV20 * b, a.mV21 * b, a.mV22 * b, a.mV23 * b, 
                                 a.mV30 * b, a.mV31 * b, a.mV32 * b, a.mV33 * b);
        }
        friend Matrix44 operator /(const Matrix44 &a, double b)
        {
            return Matrix44(a.mV00 / b, a.mV01 / b, a.mV02 / b, a.mV03 / b,
                                 a.mV10 / b, a.mV11 / b, a.mV12 / b, a.mV13 / b,
                                 a.mV20 / b, a.mV21 / b, a.mV22 / b, a.mV23 / b,
                                 a.mV30 / b, a.mV31 / b, a.mV32 / b, a.mV33 / b);
        }

        friend Matrix44 Abs(const Matrix44 &a)
        {
            return Matrix44(fabs(a.mV00), fabs(a.mV01), fabs(a.mV02), fabs(a.mV03),
                                 fabs(a.mV10), fabs(a.mV11), fabs(a.mV12), fabs(a.mV13),
                                 fabs(a.mV20), fabs(a.mV21), fabs(a.mV22), fabs(a.mV23),
                                 fabs(a.mV30), fabs(a.mV31), fabs(a.mV32), fabs(a.mV33));
        }

        friend Matrix44 operator *(double b, const Matrix44 &a)
        {
            return Matrix44(a.mV00 * b, a.mV01 * b, a.mV02 * b, a.mV03 * b,
                                 a.mV10 * b, a.mV11 * b, a.mV12 * b, a.mV13 * b,
                                 a.mV20 * b, a.mV21 * b, a.mV22 * b, a.mV23 * b,
                                 a.mV30 * b, a.mV31 * b, a.mV32 * b, a.mV33 * b);
        }
        friend Matrix44 operator *(const Matrix44 &a, const Matrix44 &b)
        {

            return Matrix44(a.mV00 * b.mV00 + a.mV01 * b.mV10 + a.mV02 * b.mV20 + a.mV03 * b.mV30,
                                 a.mV00 * b.mV01 + a.mV01 * b.mV11 + a.mV02 * b.mV21 + a.mV03 * b.mV31,
                                 a.mV00 * b.mV02 + a.mV01 * b.mV12 + a.mV02 * b.mV22 + a.mV03 * b.mV32,
                                 a.mV00 * b.mV03 + a.mV01 * b.mV13 + a.mV02 * b.mV23 + a.mV03 * b.mV33,

                                 a.mV10 * b.mV00 + a.mV11 * b.mV10 + a.mV12 * b.mV20 + a.mV13 * b.mV30,
                                 a.mV10 * b.mV01 + a.mV11 * b.mV11 + a.mV12 * b.mV21 + a.mV13 * b.mV31,
                                 a.mV10 * b.mV02 + a.mV11 * b.mV12 + a.mV12 * b.mV22 + a.mV13 * b.mV32,
                                 a.mV10 * b.mV03 + a.mV11 * b.mV13 + a.mV12 * b.mV23 + a.mV13 * b.mV33,

                                 a.mV20 * b.mV00 + a.mV21 * b.mV10 + a.mV22 * b.mV20 + a.mV23 * b.mV30,
                                 a.mV20 * b.mV01 + a.mV21 * b.mV11 + a.mV22 * b.mV21 + a.mV23 * b.mV31,
                                 a.mV20 * b.mV02 + a.mV21 * b.mV12 + a.mV22 * b.mV22 + a.mV23 * b.mV32,
                                 a.mV20 * b.mV03 + a.mV21 * b.mV13 + a.mV22 * b.mV23 + a.mV23 * b.mV33,
                                 
                                 a.mV30 * b.mV00 + a.mV31 * b.mV10 + a.mV32 * b.mV20 + a.mV33 * b.mV30,
                                 a.mV30 * b.mV01 + a.mV31 * b.mV11 + a.mV32 * b.mV21 + a.mV33 * b.mV31,
                                 a.mV30 * b.mV02 + a.mV31 * b.mV12 + a.mV32 * b.mV22 + a.mV33 * b.mV32,
                                 a.mV30 * b.mV03 + a.mV31 * b.mV13 + a.mV32 * b.mV23 + a.mV33 * b.mV33);
        }

        friend Matrix44 Transpose(const Matrix44 &M)
        {
            return Matrix44(M.mV00, M.mV10, M.mV20, M.mV30,
                                 M.mV01, M.mV11, M.mV21, M.mV31,
                                 M.mV02, M.mV12, M.mV22, M.mV32,
                                 M.mV03, M.mV13, M.mV23, M.mV33);
        }

        friend Vector4 operator *(const Matrix44 &M, const Vector4 &p)
        {
            Vector4 result;

            result.x = M.mV00 * p.x + M.mV01 * p.y + M.mV02 * p.z + M.mV03 * p.w;
            result.y = M.mV10 * p.x + M.mV11 * p.y + M.mV12 * p.z + M.mV13 * p.w;
            result.z = M.mV20 * p.x + M.mV21 * p.y + M.mV22 * p.z + M.mV23 * p.w;
            result.w = M.mV30 * p.x + M.mV31 * p.y + M.mV32 * p.z + M.mV33 * p.w;

            return result;

        }
        friend Vector4 operator *(const Vector4 &p, const Matrix44 &M)
        {
            Vector4 result;

            result.x = M.mV00 * p.x + M.mV10 * p.y + M.mV20 * p.z + M.mV30 * p.w;
            result.y = M.mV01 * p.x + M.mV11 * p.y + M.mV21 * p.z + M.mV31 * p.w;
            result.z = M.mV02 * p.x + M.mV12 * p.y + M.mV22 * p.z + M.mV32 * p.w;
            result.w = M.mV03 * p.x + M.mV13 * p.y + M.mV23 * p.z + M.mV33 * p.w;

            return result;

        }

        friend Vector3 operator *(const Matrix44 &M, const Vector3 &p)
        {
            Vector3 result;

            //double w = M.mV30 * p.x + M.mV31 * p.y + M.mV32 * p.z + M.mV33;
            //result.x = (M.mV00 * p.x + M.mV01 * p.y + M.mV02 * p.z + M.mV03) / w;
            //result.y = (M.mV10 * p.x + M.mV11 * p.y + M.mV12 * p.z + M.mV13) / w;
            //result.z = (M.mV20 * p.x + M.mV21 * p.y + M.mV22 * p.z + M.mV23) / w;

            result.x = (M.mV00 * p.x + M.mV01 * p.y + M.mV02 * p.z + M.mV03);
            result.y = (M.mV10 * p.x + M.mV11 * p.y + M.mV12 * p.z + M.mV13);
            result.z = (M.mV20 * p.x + M.mV21 * p.y + M.mV22 * p.z + M.mV23);
            return result;

        }

        friend Vector3 operator *(const Vector3 &p, const Matrix44 &M)
        {
            Vector3 result;

            //double w = M.mV03 * p.x + M.mV13 * p.y + M.mV23 * p.z + M.mV33;
            //result.x = (M.mV00 * p.x + M.mV10 * p.y + M.mV20 * p.z + M.mV30)/w ;
            //result.y = (M.mV01 * p.x + M.mV11 * p.y + M.mV21 * p.z + M.mV31)/w ;
            //result.z = (M.mV02 * p.x + M.mV12 * p.y + M.mV22 * p.z + M.mV32)/w ;

            result.x = (M.mV00 * p.x + M.mV10 * p.y + M.mV20 * p.z + M.mV30);
            result.y = (M.mV01 * p.x + M.mV11 * p.y + M.mV21 * p.z + M.mV31);
            result.z = (M.mV02 * p.x + M.mV12 * p.y + M.mV22 * p.z + M.mV32);
            
            return result;

        }

        void MakeIdentity()
        {
            mV00 = 1.0;
            mV01 = 0.0;
            mV02 = 0.0;
            mV03 = 0.0;

            mV10 = 0.0;
            mV11 = 1.0;
            mV12 = 0.0;
            mV13 = 0.0;

            mV20 = 0.0;
            mV21 = 0.0;
            mV22 = 1.0;
            mV23 = 0.0;

            mV30 = 0.0;
            mV31 = 0.0;
            mV32 = 0.0;
            mV33 = 1.0;

        }

        Matrix33 ExtractRotation()
        {
            return Matrix33(mV00, mV01, mV02, mV10, mV11, mV12, mV20, mV21, mV22);
        }
        Vector3 ExtractTranslation()
        {
            return Vector3(mV03, mV13, mV23);
        }

        private:
            double mV00, mV01, mV02, mV03;
            double mV10, mV11, mV12, mV13;
            double mV20, mV21, mV22, mV23;
            double mV30, mV31, mV32, mV33;
    };

}
