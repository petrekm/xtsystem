namespace Microsoft
{
    class HResult
    {
    public:
        HResult(HRESULT ahr=S_OK)
        {
            data = ahr;
        }
        XTSystem::String ToString() const
        {
            const std::vector<HRitem> &all = GetHRitems();
            for (std::vector<HRitem>::const_iterator it = all.cbegin(); it != all.cend(); ++it)
            {
                if (data == it->hr) return it->Name;
            }

            return XTSystem::String::Format(L"UNKNOWN_HRESULT_0x{0:x}",(unsigned long)data);
        }

        XTSystem::String Description() const
        {
            const std::vector<HRitem> &all = GetHRitems();
            for (std::vector<HRitem>::const_iterator it = all.cbegin(); it != all.cend(); ++it)
            {
                if (data == it->hr) return it->Description;
            }

            return L"Unknown description";
        }
        operator HRESULT() { return data; }
        operator unsigned long() { return (unsigned long)data; }
        unsigned long ToULong() const { return (unsigned long) data; }
        long ToLong() const { return (long)data; }

        HRESULT data;

    private:
        struct HRitem
        {
            HRitem(HRESULT ahr, const XTSystem::Char* aName, const XTSystem::Char* aDescription)
            {
                hr = ahr;
                Name = aName;
                Description = aDescription;
            }

            HRESULT hr;
            const XTSystem::Char*Name;
            const XTSystem::Char*Description;
        };

        static const std::vector<HRitem> &GetHRitems() 
        {
            static std::vector<HRitem> items;
#define HRITEM(HR,DESC) items.push_back(HRitem(HR,_T(#HR), DESC));
#define HRITEM2(DESC,HR,CODE) items.push_back(HRitem(CODE,_T(#HR), DESC));

            CALL_ONCE_BLOCK
            {
                HRITEM(S_OK, L"Operation successful")
                HRITEM(E_ABORT, L"Operation aborted")
                HRITEM(E_ACCESSDENIED, L"General access denied error")
                HRITEM(E_FAIL, L"Unspecified failure")
                HRITEM(E_HANDLE, L"Handle that is not valid")
                HRITEM(E_INVALIDARG, L"One or more arguments are not valid")
                HRITEM(E_NOINTERFACE, L"No such interface supported")
                HRITEM(E_NOTIMPL, L"Not implemented")
                HRITEM(E_OUTOFMEMORY, L"Failed to allocate necessary memory")
                HRITEM(E_POINTER, L"Pointer that is not valid")
                HRITEM(E_UNEXPECTED, L"Unexpected failure")

                HRITEM2(L"Software configuration problem", E_CONFIGURATION, 0x80040810)
                HRITEM2(L"Method execution was cancelled.", E_CANCEL, 0x80040811)
                HRITEM2(L"Return codes related to synchronous asynchronous calltype", E_SYNCHRONOUS_NOTIMPL, 0x80040812)
                HRITEM2(L"Return codes related to synchronous asynchronous calltype", E_ASYNCHRONOUS_NOTIMPL, 0x80040813)
                HRITEM2(L"Return codes related to synchronous asynchronous calltype", E_SYNCHRONOUS_NOTALLOWED, 0x80040814)
                HRITEM2(L"Return codes related to synchronous asynchronous calltype", E_ASYNCHRONOUS_NOTALLOWED, 0x80040815)
                HRITEM2(L"The method could not be executed because a (critical) resource could not be acquired.", E_OUTOFRESOURCES, 0x80040820)
                HRITEM2(L"The method could not be executed because a (critical) resource could not be acquired.", E_NORESOURCE, 0x80040821)
                HRITEM2(L"The requested state transition could not be done in the current state", E_NOTCONTROLLABLE, 0x80040830)
                HRITEM2(L"The requested start action could not be done in the current state", E_NOTSTARTABLE, 0x80040831)
                HRITEM2(L"The requested cancel action could not be done in the current state", E_NOTCANCELLABLE, 0x80040832)
                HRITEM2(L"The requested abortaction could not be done in the current state", E_NOTABORTABLE, 0x80040833)
                HRITEM2(L"The method call is not allowed because the module is not in a state it can process the request.", E_NOTALLOWED, 0x80040834)
                HRITEM2(L"There was a problem with an internal state transition.", E_STATETRANSITION, 0x80040835)
                HRITEM2(L"The requested method tried to put the component in a state that cannot be reached from the current state.", E_INVALIDTARGETSTATE, 0x80040836)
                HRITEM2(L"The requested object has no instance", E_NOINSTANCE, 0x80040837)
                HRITEM2(L"Method failed to initialize.", E_INITIALIZE, 0x80040840)
                HRITEM2(L"Method execution took too long.", E_TIMEOUT, 0x80040841)
                HRITEM2(L"Method could not be executed because of a hardware failure.", E_HARDWAREFAILURE, 0x80040842)
                HRITEM2(L"Method could not be executed because the component is offline.", E_OFFLINE, 0x80040843)
                HRITEM2(L"Method could not be executed because the component is in a global error state.", E_ERRORSTATE, 0x80040844)
                HRITEM2(L"One of the conditions related to succesfull execution of the method could not be met. This return value is related to methods that can first try to get their own preconditions right automatically.", E_CONDITIONSNOTMET, 0x80040845)
                HRITEM2(L"One of the conditions related to succesfull execution of the method could not be met. This return value is related to methods that can first try to get their own preconditions right automatically.", E_CONDITIONFAILED, 0x80040846)
                HRITEM2(L"Target value is set (cached) but the set of the actual value is postponed", E_CACHEDTARGETVALUE, 0x80040847)

            }
#undef HRITEM
#undef HRITEM2
            return items;
        }

    };
}

