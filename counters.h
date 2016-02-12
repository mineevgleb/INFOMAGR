#pragma once

void InitPerformanceCounters();
void StartMeasurement();
void StopMeasurement();
int GetNrCounters();
char* GetCounterName( int idx );
int GetCounterValue( int idx );

// maximum number of threads. Must be 4 or 8.
#if defined(_M_X64) || defined(__x86_64__) || defined(__amd64)
#define MAXTHREADS  8
#else
#define MAXTHREADS  4
#endif

// list of input/output data structures for MSR driver
#define MAX_QUE_ENTRIES 32                  // maximum number of entries in queue

// commands for MSR driver. Shared with application program
enum EMSR_COMMAND 
{
    MSR_IGNORE = 0,                // do nothing
    MSR_STOP   = 1,                // skip rest of list
    MSR_READ   = 2,                // read model specific register
    MSR_WRITE  = 3,                // write model specific register
    CR_READ    = 4,                // read control register
    CR_WRITE   = 5,                // write control register
    PMC_ENABLE = 6,                // Enable RDPMC and RDTSC instructions
    PMC_DISABLE= 7,                // Disable RDPMC instruction (RDTSC remains enabled)
    PROC_GET   = 8,                // Get processor number (In multiprocessor systems. 0-based)
    PROC_SET   = 9,                // Set processor number (In multiprocessor systems. 0-based)
    UNUSED1    = 0x7fffffff        // make sure this enum takes 32 bits
};

// input/output data structure for MSR driver
struct SMSRInOut 
{
    enum EMSR_COMMAND msr_command;  // command for read or write register
    unsigned int register_number;	// register number
    union 
	{
        long long value;            // 64 bit value to read or write
        unsigned int val[2];        // lower and upper 32 bits
    };
};

// maximum number of performance counters used
const int MAXCOUNTERS = 8;

// maximum number of repetitions
const int MAXREPEAT = 128;

// max name length of counters
const int COUNTERNAMELEN = 10; 

class CMSRInOutQue 
{
public:
    // constructor
    CMSRInOutQue();
    // put record in queue
    int put( EMSR_COMMAND msr_command, unsigned int register_number, unsigned int value_lo, unsigned int value_hi = 0 );
    // list of entries
    SMSRInOut queue[MAX_QUE_ENTRIES+1];
    // get size of queue
    int GetSize() {return n;}
protected:
    // number of entries
    int n;
};

//////////////////////////////////////////////////////////////////////////////
//  Definitions due to different compiler syntax
//////////////////////////////////////////////////////////////////////////////

// define 64 bit integer
typedef __int64 int64;
typedef unsigned __int64 uint64;

#ifndef _MSC_VER
// Define macro for aligned structure, gcc syntax
#define ALIGNEDSTRUCTURE(Name, Align) struct __attribute__((aligned(Align))) Name
#else
// Define macro for aligned structure, MS Visual Studio syntax
#define ALIGNEDSTRUCTURE(Name, Align) __declspec(align(Align)) struct Name
#endif

// Define low level functions
#ifdef _MSC_VER // __INTRIN_H_  // Use intrinsics for low level functions

static volatile int DontSkip;
static inline void Serialize () 
{
    int dummy[4];
    __cpuid(dummy, 0);
    DontSkip = dummy[0];
}
#define Cpuid __cpuid
#define Readtsc __rdtsc
#define Readpmc __readpmc

#else // use gcc style inline assembly

static void Cpuid (int Output[4], int aa) 
{	
	int a, b, c, d;
	__asm("cpuid" : "=a"(a),"=b"(b),"=c"(c),"=d"(d) : "a"(aa),"c"(0) : );
	Output[0] = a;
	Output[1] = b;
	Output[2] = c;
	Output[3] = d;
}

static inline void Serialize() 
{
	__asm__ __volatile__ ( "xorl %%eax, %%eax \n cpuid " : : : "%eax","%ebx","%ecx","%edx" );
}

static inline int Readtsc() 
{
	int r;
	__asm__ __volatile__ ( "rdtsc" : "=a"(r) : : "%edx");    
	return r;
}

static inline int Readpmc(int nPerfCtr) 
{
	int r;
	__asm__ __volatile__ ( "rdpmc" : "=a"(r) : "c"(nPerfCtr) : "%edx");    
	return r;
}

#endif  // _MSC_VER 

//////////////////////////////////////////////////////////////////////////////
//  Definitions due to different OS calls
//////////////////////////////////////////////////////////////////////////////

namespace SyS 
{
	typedef DWORD_PTR ProcMaskType;          // Type for processor mask
	static inline ProcMaskType GetProcessMask() 
	{
		ProcMaskType ProcessAffMask = 0, SystemAffMask = 0;
		GetProcessAffinityMask( GetCurrentProcess(), &ProcessAffMask, &SystemAffMask );
		return ProcessAffMask;
	}
	static inline void SetProcessMask( int p ) 
	{
		SetThreadAffinityMask( GetCurrentThread(), (ProcMaskType)1 << p );   
	}
	static inline int TestProcessMask( int p, ProcMaskType* m ) 
	{
		return ((ProcMaskType)1 << p) & *m;
	}
	static inline void Sleep0() { Sleep( 0 ); }
	static inline void SetProcessPriorityHigh() 
	{
		SetPriorityClass( GetCurrentProcess(), REALTIME_PRIORITY_CLASS );
	}
	static inline void SetProcessPriorityNormal() 
	{
		SetPriorityClass( GetCurrentProcess(), NORMAL_PRIORITY_CLASS );
	}
}

//////////////////////////////////////////////////////////////////////
//                         class CMSRDriver
//////////////////////////////////////////////////////////////////////

class CMSRDriver 
{
protected:
	SC_HANDLE scm, service;
	HANDLE hDriver;
	const char* DriverFileName, *DriverSymbolicName;
	char DriverFileNameE[MAX_PATH], DriverFilePath[MAX_PATH];
public:
	CMSRDriver() 
	{
		if (Need64BitDriver()) DriverFileName = "MSRDriver64"; else DriverFileName = "MSRDriver32";
		DriverSymbolicName = "\\\\.\\slMSRDriver";
		strcpy( DriverFileNameE, DriverFileName );
		strcat( DriverFileNameE, ".sys" );
		::GetFullPathName( DriverFileNameE, MAX_PATH, DriverFilePath, NULL );
		service = scm = NULL, hDriver = NULL;
	}
	~CMSRDriver() 
	{
		if (service) { ::CloseServiceHandle( service ); service = NULL; }
		if (scm) { ::CloseServiceHandle( scm ); scm = NULL; }
	}
    const char* GetDriverName() { return DriverFileName; }
    int LoadDriver() 
	{
		int r = 0, e = 0;
		// open driver service
		// UnInstallDriver(); // uncomment if you need to uninstall/reboot/reinstall
		// InstallDriver(); // uncomment after uninstall/reboot
		r = OpenDriver();
		if (r == 1060) 
		{
			// Driver not installed. Install it
			e = InstallDriver();
			if (e) return e; else r = OpenDriver();
		}
		if (r) { printf("\nError %i loading driver\n", r); return r; }
		r = ::StartService(service, 0, NULL);
		if (r == 0) 
		{
			e = ::GetLastError();
			switch (e) 
			{
			case ERROR_PATH_NOT_FOUND:
				printf( "\nDriver file %s path not found (please try to uninstall and reinstall)\n", DriverFileNameE );
				break;
			case ERROR_FILE_NOT_FOUND:  // .sys file not found
				printf( "\nDriver file %s not found\n", DriverFileNameE );
				break;
			case 577:
				printf( "\nThe driver %s is not signed by Microsoft\nPlease press F8 during boot and select 'Disable Driver Signature Enforcement'\n", DriverFileNameE );
				break;
			case 1056: e = 0; break;
			case 1058: printf( "\nError: Driver disabled\n" ); break;
			default: printf( "\nCannot load driver %s\nError no. %i", DriverFileNameE, e );
			}
		}
		if (e == 0) 
		{
			hDriver = ::CreateFile( DriverSymbolicName, GENERIC_READ + GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );
			if(hDriver == NULL || hDriver == INVALID_HANDLE_VALUE) 
			{
				hDriver = NULL;
				e = ::GetLastError();
				printf( "\nCannot load driver\nError no. %i", e );
			}
		}
		return e;
	}
    int UnloadDriver() 
	{
        int r = 0, e = 0;
        if (GetScm() == NULL) return -6;
        if (hDriver) 
		{
            r = ::CloseHandle( hDriver ); 
			hDriver = NULL;
            if (r == 0) 
			{
                e = ::GetLastError();
                printf( "\nCannot close driver handle\nError no. %i", e );
                return e;
            }
            printf( "\nUnloading driver" );
        }
        if (service) 
		{
            SERVICE_STATUS ss;
            r = ::ControlService(service, SERVICE_CONTROL_STOP, &ss);
            if (r == 0) 
			{
                e = ::GetLastError();
                if (e == 1062) printf("\nDriver not active\n");
                          else printf("\nCannot close driver\nError no. %i", e);
                return e;
            }
        }
        return 0;
    }
protected:
    int InstallDriver() 
	{
        int e = 0;
        if (GetScm() == NULL) return -1;
        service = ::CreateService(scm, DriverFileNameE, "MSR driver",
            SERVICE_START + SERVICE_STOP + DELETE, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, 
			SERVICE_ERROR_IGNORE, DriverFilePath, NULL, NULL, NULL, NULL, NULL);
        if (service == NULL) 
		{
            e = ::GetLastError();
            printf( "\nCannot install driver %s\nError no. %i", DriverFileNameE, e );
        }
        else printf( "\nFirst time: Installing driver %s\n", DriverFileNameE );
        return e;
    } 
    int UnInstallDriver() 
	{
		int r = 0, e = 0;
		GetScm();
		if (service == 0) service = ::OpenService( scm, DriverFileNameE, SERVICE_ALL_ACCESS );
		if (service == 0) 
		{
			e = ::GetLastError();
			if (e == 1060) printf( "\nDriver %s already uninstalled or never installed\n", DriverFileNameE );
			else printf( "\nCannot open service, failed to uninstall driver %s\nError no. %i", DriverFileNameE, e );
		}
		else 
		{
			r = ::DeleteService(service);
			if (r == 0) 
			{
				e = ::GetLastError();
				printf( "\nFailed to uninstall driver %s\nError no. %i", DriverFileNameE, e );
				if (e == 1072) printf( "\nDriver already marked for deletion\n" );
			}
			else printf( "\nUninstalling driver %s\n", DriverFileNameE );
			r = ::CloseServiceHandle( service );
			if (r == 0) 
			{
				e = ::GetLastError();
				printf( "\nCannot close service\nError no. %i", e );
			}
			service = NULL;
		}
		return e;
    }
    SC_HANDLE GetScm() 
	{
		if (scm) return scm;  // handle already made
		scm = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
		if (scm == NULL) 
		{
			int e = ::GetLastError();
			if (e == ERROR_ACCESS_DENIED) printf( "\nAccess denied. Please run as administrator\n" );
			else if (e == 120) printf( "\nFunction not implemented on this operating system. Windows 2000 or later required.\n" );
			else printf( "\nCannot load Windows Service Control Manager\nError no. %i", e );
		}
		return scm;
    }
    int OpenDriver() 
	{
		if (service == 0) service = ::OpenService( GetScm(), DriverFileNameE, SERVICE_ALL_ACCESS );
		if (service == 0) 
		{
			int e = ::GetLastError();
			switch (e) 
			{
			case 1060: break;
			case 6: printf( "\nAccess denied\n" ); break;
			default: printf( "\nCannot open service, failed to load driver %s\nError no. %i", DriverFileNameE, e );
			}
			return e;
		}
		return 0;
    }
    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
    int Need64BitDriver() 
	{
	#ifdef _WIN64
		return 2;
	#else
		LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress( GetModuleHandle( "kernel32" ),"IsWow64Process" );   
		if (fnIsWow64Process) 
		{
			BOOL bIsWow64 = FALSE;      
			if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64)) return 0;
			return bIsWow64;
		}
		return 0;
	#endif
    }
public:
    int AccessRegisters( void* pnIn, int nInLen, void* pnOut, int nOutLen ) 
	{
		if (nInLen <= 0) return 0;
		const int DeviceType = 0x22;        // FILE_DEVICE_UNKNOWN;
		const int Function = 0x800;
		const int Method = 0;               // METHOD_BUFFERED;
		const int Access = 1 | 2;           // FILE_READ_ACCESS | FILE_WRITE_ACCESS;
		const int IOCTL_MSR_DRIVER = DeviceType << 16 | Access << 14 | Function << 2 | Method;
		DWORD len = 0;
		int res = ::DeviceIoControl(hDriver, IOCTL_MSR_DRIVER, pnIn, nInLen, pnOut, nOutLen, &len, NULL);
		if (!res) 
		{ 
			int e = GetLastError();
			printf("\nCan't access driver. error %i", e);
			return e;
		}
		SMSRInOut * outp = (SMSRInOut*)pnOut;
		for( int i = 0; i < nOutLen / (INT)sizeof(SMSRInOut); i++ ) 
			if (outp[i].msr_command == PROC_SET && outp[i].val[0])
				printf( "\nSetting processor number in driver failed, error 0x%X", outp[i].val[0] );
		return 0;
    }
    int AccessRegisters( CMSRInOutQue& q ) 
	{
		int n = q.GetSize() * sizeof(SMSRInOut);  
		if (n <= 0) return 0;
		return AccessRegisters( q.queue, n, q.queue, n );
    }
    int64 MSRRead( int r )
	{
		SMSRInOut a;
		a.msr_command = MSR_READ, a.register_number = r, a.value = 0;
		AccessRegisters( &a,sizeof(a), &a, sizeof(a) );
		return a.val[0];
    } 
	int MSRWrite( int r, int64 val )
	{
		SMSRInOut a;
		a.msr_command = MSR_WRITE, a.register_number = r, a.value = val;
		return AccessRegisters( &a, sizeof(a), &a, sizeof(a) );
    }
    size_t CRRead( int r )
	{
		if (r != 0 && r != 4) return -11;
		SMSRInOut a;
		a.msr_command = CR_READ;
		a.register_number = r;
		a.value = 0;
		AccessRegisters( &a, sizeof(a), &a, sizeof(a) );
		return size_t(a.value);
    }
	int CRWrite( int r, size_t val ) 
	{
		if (r != 0 && r != 4) return -12;
		SMSRInOut a;
		a.msr_command = CR_WRITE, a.register_number = r, a.value = val;
		return AccessRegisters( &a, sizeof(a), &a, sizeof(a) );
    } 
};

enum EProcVendor { VENDOR_UNKNOWN = 0, INTEL, AMD, VIA };

enum EProcFamily 
{
	PRUNKNOWN    = 0,                       // unknown. cannot do performance monitoring
	PRALL        = 0xFFFFFFFF,              // All processors with the specified scheme (Renamed. Previous name P_ALL clashes with <sys/wait.h>)
	INTEL_P1MMX  = 1,                       // Intel Pentium 1 or Pentium MMX
	INTEL_P23    = 2,                       // Pentium Pro, Pentium 2, Pentium 3
	INTEL_PM     = 4,                       // Pentium M, Core, Core 2
	INTEL_P4     = 8,                       // Pentium 4 and Pentium 4 with EM64T
	INTEL_CORE   = 0x10,                    // Intel Core Solo/Duo
	INTEL_P23M   = 0x16,                    // Pentium Pro, Pentium 2, Pentium 3, Pentium M, Core1
	INTEL_CORE2  = 0x20,                    // Intel Core 2
	INTEL_7      = 0x40,                    // Intel Core i7, Nehalem, Sandy Bridge
	INTEL_IVY    = 0x80,                    // Intel Ivy Bridge
	INTEL_7I     = 0xC0,                    // Nehalem, Sandy Bridge, Ivy bridge
	INTEL_HASW   = 0x100,                   // Intel Haswell and later
	INTEL_ATOM   = 0x1000,                  // Intel Atom
	INTEL_SILV   = 0x2000,                  // Intel Silvermont
	AMD_ATHLON   = 0x10000,                 // AMD Athlon
	AMD_ATHLON64 = 0x20000,                 // AMD Athlon 64 or Opteron
	AMD_BULLD    = 0x80000,                 // AMD Family 15h (Bulldozer)
	AMD_ALL      = 0xF0000,                 // AMD any processor
	VIA_NANO     = 0x100000,                // VIA Nano (Centaur)
};

enum EPMCScheme 
{
	S_UNKNOWN = 0,                           // unknown. can't do performance monitoring
	S_P1   = 0x0001,                         // Intel Pentium, Pentium MMX
	S_P4   = 0x0002,                         // Intel Pentium 4, Netburst
	S_P2   = 0x0010,                         // Intel Pentium 2, Pentium M
	S_ID1  = 0x0020,                         // Intel Core solo/duo
	S_ID2  = 0x0040,                         // Intel Core 2
	S_ID3  = 0x0080,                         // Intel Core i7 and later and Atom
	S_P2MC = 0x0030,                         // Intel Pentium 2, Pentium M, Core solo/duo
	S_ID23 = 0x00C0,                         // Intel Core 2 and later
	S_INTL = 0x00F0,                         // Most Intel schemes
	S_AMD  = 0x1000,                         // AMD processors
	S_VIA  = 0x2000                          // VIA Nano processor and later
};

struct SCounterDefinition 
{
	int          CounterType;                // ID identifying what to count
	EPMCScheme   PMCScheme;                  // PMC scheme. values may be OR'ed
	EProcFamily  ProcessorFamily;            // processor family. values may be OR'ed
	int          CounterFirst, CounterLast;  // counter number or a range of possible alternative counter numbers
	int          EventSelectReg;             // event select register
	int          Event;                      // event code
	int          EventMask;                  // event mask
	char         Description[COUNTERNAMELEN];// name of counter. length must be < COUNTERNAMELEN
};

class CCounters 
{
public:
	CCounters();                             // constructor
	const char* DefineCounter( int CounterType );   // request a counter setup
	const char* DefineCounter( SCounterDefinition& CounterDef ); // request a counter setup
	void LockProcessor();                    // Make program and driver use the same processor number
	void QueueCounters();                    // Put counter definitions in queue
	int  StartDriver();                      // Install and load driver
	void PrintCPUInfo();					 // Print CPU info
	void StartCounters( int ThreadNum );     // start counting
	void StopCounters ( int ThreadNum );     // stop and reset counters
	void CleanUp();                          // Any required cleanup of driver etc
	CMSRDriver msr;                          // interface to MSR access driver
	char* CounterNames[MAXCOUNTERS];         // name of each counter
	void Put1( int num_threads,              // put record into multiple start queues
		EMSR_COMMAND msr_command, unsigned int register_number,
		unsigned int value_lo, unsigned int value_hi = 0 );
	void Put2( int num_threads,              // put record into multiple stop queues
		EMSR_COMMAND msr_command, unsigned int register_number,
		unsigned int value_lo, unsigned int value_hi = 0 );
	void GetProcessorVendor();               // get microprocessor vendor
	void GetProcessorFamily();               // get microprocessor family
	void GetPMCScheme();                     // get PMC scheme
	protected:
	CMSRInOutQue queue1[64];                 // que of MSR commands to do by StartCounters()
	CMSRInOutQue queue2[64];                 // que of MSR commands to do by StopCounters()
	// translate event select number to register address for P4 processor:
	static int GetP4EventSelectRegAddress( int CounterNr, int EventSelectNo ); 
	int NumCounterDefinitions;               // number of possible counter definitions in table CounterDefinitions
	EProcVendor MVendor;                     // microprocessor vendor
	EProcFamily MFamily;                     // microprocessor type and family
	EPMCScheme  MScheme;                     // PMC monitoring scheme
	int NumPMCs;                             // Number of general PMCs
	int NumFixedPMCs;                        // Number of fixed function PMCs
	int ProcessorNumber;                     // main thread processor number in multiprocessor systems
};