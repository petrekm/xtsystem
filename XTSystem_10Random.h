namespace XTSystem 
{
	  class Random 
	  {
	  private:
			int MBIG;
			int MSEED;
			int MZ;
			
      
			int inext;
			int inextp;
			int SeedArray[56];
    
	  void InitConsts()
	  {
			MBIG =  Int32::MaxValue();
			MSEED = 161803398;
			MZ = 0;

			inext = 0;
			inextp = 0;
	  }
	  public:
	 Random()
	 {
		 MakeRandom(GetTickCount());
	 }
    
	 Random(int Seed)
	 {
		 MakeRandom(Seed);
	 }

	  private: void MakeRandom(int Seed) 
	  {
		  InitConsts();
        int ii;
        int mj, mk;
    
        int subtraction = (Seed == Int32::MinValue()) ? Int32::MaxValue() : Math::Abs(Seed);
        mj = MSEED - subtraction;
        SeedArray[55]=mj;
        mk=1;
        for (int i=1; i<55; i++) 
		{  //Apparently the range [1..55] is special (Knuth) and so we're wasting the 0'th position.
          ii = (21*i)%55;
          SeedArray[ii]=mk;
          mk = mj - mk;
          if (mk<0) mk+=MBIG;
          mj=SeedArray[ii];
        }
        for (int k=1; k<5; k++) {
          for (int i=1; i<56; i++) {
        SeedArray[i] -= SeedArray[1+(i+30)%55];
        if (SeedArray[i]<0) SeedArray[i]+=MBIG;
          }
        }
        inext=0;
        inextp = 21;
        Seed = 1;
      }
    
	  protected: virtual double Sample() 
	  {
          //Including this division at the end gives us significantly improved
          //random number distribution.
          return (InternalSample()*(1.0/MBIG));
      }
    
	  private: int InternalSample() 
	  {
          int retVal;
          int locINext = inext;
          int locINextp = inextp;
 
          if (++locINext >=56) locINext=1;
          if (++locINextp>= 56) locINextp = 1;
          
          retVal = SeedArray[locINext]-SeedArray[locINextp];
 
          if (retVal == MBIG) retVal--;          
          if (retVal<0) retVal+=MBIG;
          
          SeedArray[locINext]=retVal;
 
          inext = locINext;
          inextp = locINextp;
                    
          return retVal;
      }

	  public: virtual int Next() 
	  {
        return InternalSample();
      }
    
	  private: double GetSampleForLargeRange() 
			   {
 
          int result = InternalSample();
          // Note we can't use addition here. The distribution will be bad if we do that.
          bool negative = (InternalSample()%2 == 0) ? true : false;  // decide the sign based on second sample
          if( negative) {
              result = -result;
          }
          double d = result;
          d += (Int32::MaxValue() - 1); // get a number in range [0 .. 2 * Int32MaxValue - 1)
          d /= 2*(unsigned int)Int32::MaxValue() - 1  ;              
          return d;
      }
 
 
	  public: virtual int Next(int minValue, int maxValue) 
	  {
          if (minValue>maxValue) {
              //throw new ArgumentOutOfRangeException("minValue",Environment.GetResourceString("Argument_MinMaxValue", "minValue", "maxValue"));
          }
          //Contract.EndContractBlock();
          
          long range = (long)maxValue-minValue;
          if( range <= (long)Int32::MaxValue()) {  
              return ((int)(Sample() * range) + minValue);
          }          
          else { 
              return (int)((long)(GetSampleForLargeRange() * range) + minValue);
          }
      }
    
    
	  public: virtual int Next(int maxValue) 
	  {
          if (maxValue<0) 
		  {
              //throw new ArgumentOutOfRangeException("maxValue", Environment.GetResourceString("ArgumentOutOfRange_MustBePositive", "maxValue"));
          }
          //Contract.EndContractBlock();
          return (int)(Sample()*maxValue);
      }
    
    
	  public: virtual double NextDouble() 
	  {
        return Sample();
      }
    
    
	  //public: virtual void NextBytes(byte [] buffer)
	  //{
   //     if (buffer==null) throw new ArgumentNullException("buffer");
   //     //Contract.EndContractBlock();
   //     for (int i=0; i<buffer.Length; i++) {
   //       buffer[i]=(byte)(InternalSample()%(Byte.MaxValue+1)); 
   //     }
   //   }
    };
 

} 

/*
namespace XTSystem
{
    class MT19937
    {
        // Period parameters
        private:
        const ulong    N                = 624;
        const ulong    M                = 397;
        const ulong    MATRIX_A        = 0x9908B0DFUL;        // constant vector a
        const ulong UPPER_MASK        = 0x80000000UL;        // most significant w-r bits
        const ulong LOWER_MASK        = 0X7FFFFFFFUL;        // least significant r bits
        const uint    DEFAULT_SEED    = 4357;

        static ulong [] mt            = new ulong[N+1];    // the array for the state vector
        static ulong    mti            = N + 1;            // mti==N+1 means mt[N] is not initialized

        public: MT19937()
        {
            ulong [] init = new ulong[4];
            init[0]= 0x123;
            init[1]= 0x234;
            init[2]= 0x345;
            init[3] =0x456;
            ulong length = 4;
            init_by_array(init, length);
        }

        // initializes mt[N] with a seed
        void init_genrand(ulong s)
        {
            mt[0]= s & 0xffffffffUL;
            for (mti=1; mti < N; mti++)
            {
                mt[mti] = (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
                // See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier.
                // In the previous versions, MSBs of the seed affect
                // only MSBs of the array mt[].
                // 2002/01/09 modified by Makoto Matsumoto
                mt[mti] &= 0xffffffffUL;
                // for >32 bit machines
            }
        }


        // initialize by an array with array-length
        // init_key is the array for initializing keys
        // key_length is its length
        public void init_by_array(ulong[] init_key, ulong key_length)
        {
            ulong i, j, k;
            init_genrand(19650218UL);
            i=1; j=0;
            k = ( N > key_length ? N : key_length);
            for (; k > 0; k--)
            {
                mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
                + init_key[j] + j;        // non linear
                mt[i] &= 0xffffffffUL;    // for WORDSIZE > 32 machines
                i++; j++;
                if (i>=N) { mt[0] = mt[N-1]; i=1; }
                if (j>=key_length) j=0;
            }
            for (k = N - 1; k > 0; k--)
            {
                mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
                - i;                    // non linear
                mt[i] &= 0xffffffffUL;    // for WORDSIZE > 32 machines
                i++;
                if (i>=N) { mt[0] = mt[N-1]; i=1; }
            }
            mt[0] = 0x80000000UL;        // MSB is 1; assuring non-zero initial array
        }

        // generates a random number on [0,0x7fffffff]-interval
        public long genrand_int31()
        {
            return (long)(genrand_int32()>>1);
        }
        // generates a random number on [0,1]-real-interval
        public double genrand_real1()
        {
            return (double)genrand_int32()*(1.0/4294967295.0); // divided by 2^32-1
        }
        // generates a random number on [0,1)-real-interval
        public double genrand_real2()
        {
            return (double)genrand_int32()*(1.0/4294967296.0); // divided by 2^32
        }
        // generates a random number on (0,1)-real-interval
        public double genrand_real3()
        {
            return (((double)genrand_int32()) + 0.5)*(1.0/4294967296.0); // divided by 2^32
        }
        // generates a random number on [0,1) with 53-bit resolution
        public double genrand_res53()
        {
            ulong a = genrand_int32() >>5;
            ulong b = genrand_int32()>>6;
            return(double)(a*67108864.0+b)*(1.0/9007199254740992.0);
        }
        // These real versions are due to Isaku Wada, 2002/01/09 added

        // generates a random number on [0,0xffffffff]-interval
        public ulong genrand_int32()
        {
            ulong y = 0;
            ulong [] mag01 = new ulong[2];
            mag01[0]    = 0x0UL;
            mag01[1]    = MATRIX_A;
            // mag01[x] = x * MATRIX_A  for x=0,1

            if (mti >= N)
            {
                // generate N words at one time
                ulong kk;

                if (mti == N+1)   // if init_genrand() has not been called,
                    init_genrand(5489UL); // a default initial seed is used

                for (kk=0; kk < N - M; kk++)
                {
                    y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
                    mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
                }
                for (;kk<N-1;kk++)
                {
                    y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
                    //mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
                    mt[kk] = mt[kk - 227] ^ (y >> 1) ^ mag01[y & 0x1UL];
                }
                y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
                mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

                mti = 0;
            }

            y = mt[mti++];

            // Tempering
            y ^= (y >> 11);
            y ^= (y << 7) & 0x9d2c5680UL;
            y ^= (y << 15) & 0xefc60000UL;
            y ^= (y >> 18);

            return y;
        }

        public int RandomRange(int lo, int hi)
        {
            return (Math.Abs((int)genrand_int32() % (hi - lo + 1)) + lo);
        }
        //public int RollDice(int face, int number_of_dice)
        //{
        //    int roll = 0;
        //    for(int loop=0; loop < number_of_dice; loop++)
        //    {
        //        roll += (RandomRange(1,face));
        //    }
        //    return roll;
        //}
        //public int D6(int die_count)    { return RollDice(6,die_count); }

    };
}


*/
