
namespace XTSystem
{
    namespace Diagnostics
    {
        class StopwatchObsolete
        {
        public: 
            //static const long Frequency;
            //static const bool IsHighResolution;
            StopwatchObsolete() { Reset(); }

            double ElapsedSeconds() const { return  ((double)(stop-start))/CLOCKS_PER_SEC; }

            //TimeSpan Elapsed()  { return (long) (1000*(stop-start)/CLOCKS_PER_SEC); }

            long ElapsedMilliseconds() const { return (long) ((1000*(stop-start))/CLOCKS_PER_SEC); }
            long ElapsedTicks() const { return (long) (stop-start); }
            bool IsRunning() const { return bIsRunning; }

            static long GetTimestamp() { return (long)clock(); }

            void Reset()
            {
                bIsRunning = false;
                start = 0;
                stop = 0;
            }

            void Restart()
            {
                bIsRunning = false;
                Start();
            }

            void Start()
            {
                if (!bIsRunning)
                {
                    bIsRunning = true;
                    start = stop = clock();
                }
            }

            static StopwatchObsolete StartNew()
            {
                StopwatchObsolete sw;
                sw.Start();
                return sw;
            }

            void Stop()
            {
                stop = clock();
                bIsRunning = false;

            }
        private: 
            bool bIsRunning;
            clock_t start, stop;

        };


        class Stopwatch
        {
        public: 
            typedef long long unsigned int TimeStamp;
            //static const long Frequency;
            //static const bool IsHighResolution;

            Stopwatch() 
            { 
                Reset(); 
            }
            static Stopwatch StartNew()
            {
                Stopwatch sw;
                sw.Start();
                return sw;
            }

            double ElapsedSeconds() const { return (ElapsedMicroseconds(sum)/1000000.0); }

            //TimeSpan Elapsed() 
            //{
            //
            //return (long) (1000*(stop-start)/CLOCKS_PER_SEC);
            //}

            long ElapsedMilliseconds() const { return (long)(ElapsedMicroseconds(sum)/1000); }
            //long ElapsedTicks() const { return (long) (stop-start); }
            bool IsRunning() const { return bIsRunning; }

            static TimeStamp GetTimestamp() { return GetCurrentTimestamp(); }

            void Reset()
            {
                bIsRunning = false;
                start = 0;
                stop = 0;
                sum = 0;
            }

            void Restart()
            {
                Reset();
                Start();
            }

            void Start()
            {
                if (!bIsRunning)
                {
                    bIsRunning = true;
                    start = stop = GetCurrentTimestamp();
                }
            }

            void Stop()
            {
                stop = GetCurrentTimestamp();
                sum += stop - start;
                bIsRunning = false;

            }
        private: 
            bool bIsRunning;
            TimeStamp start, stop,sum;

            static TimeStamp GetCurrentTimestamp()
            {
                LARGE_INTEGER res;
                QueryPerformanceCounter(&res);
                return res.QuadPart;
            }

            static TimeStamp InitalizeFrequency()
            {
                LARGE_INTEGER res;
                QueryPerformanceFrequency((LARGE_INTEGER *)&res);
                return res.QuadPart;
            }

            static TimeStamp ElapsedMicroseconds(TimeStamp deltaTimeStamp) 
            { 
                return ((deltaTimeStamp)* 1000000 / InitalizeFrequency()); 
            }
        };
    }
}

