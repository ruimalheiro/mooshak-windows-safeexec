#ifndef INCLUDE_WINDOWS
#define INCLUDE_WINDOWS

#include <windows.h>
#include <Aclapi.h>
#include <WinError.h>
#include <string.h>

// JOBOBJECT_BASIC_LIMIT_INFORMATION structure
// More info: http://msdn.microsoft.com/en-us/library/windows/desktop/ms684147(v=vs.85).aspx

LARGE_INTEGER PerProcessUserTimeLimitValue = { 1000000000 };
LARGE_INTEGER PerJobUserTimeLimitValue     = { 1000000000 };
DWORD         LimitFlagsValue              = JOB_OBJECT_LIMIT_PROCESS_MEMORY | 
											 JOB_OBJECT_LIMIT_JOB_MEMORY     | 
											 JOB_OBJECT_LIMIT_JOB_TIME       | 
		                                     JOB_OBJECT_LIMIT_PROCESS_TIME   |
											 JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE |
											 JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
SIZE_T        MinimumWorkingSetSizeValue   = NULL;
SIZE_T        MaximumWorkingSetSizeValue   = NULL;
DWORD         ActiveProcessLimitValue      = 10;
ULONG_PTR     AffinityValue                = NULL;
DWORD         PriorityClassValue           = NULL;
DWORD         SchedulingClassValue         = NULL;



// JOBOBJECT_EXTENDED_LIMIT_INFORMATION structure
// More info: http://msdn.microsoft.com/en-us/library/windows/desktop/ms684156(v=vs.85).aspx

SIZE_T        ProcessMemoryLimitValue    = 10000000;
SIZE_T        JobMemoryLimitValue        = 100000000;
SIZE_T        PeakProcessMemoryUsedValue = 20000000;
SIZE_T        PeakJobMemoryUsedValue     = NULL;

void usage(int argcs, char ** argv)
{
	fprintf(stderr, "\nusage: %s \"target_file\" options\n", argv[0]);
	fprintf(stderr, "options: /op1 value1 /op2 value2 ...\n\n\n");
	fprintf(stderr, "\t /processtime <Set per-process user-mode execution time limit, in 100-nanosecond ticks> (default: 10000000)\n\n");
	fprintf(stderr, "\t /jobtime <Set per-job user-mode execution time limit, in 100-nanosecond ticks> (default: 10000000)\n");
	fprintf(stderr, "\t /processmemorylimit <Set the limit for the virtual memory that can be committed by a process, in bytes> (default: 5243000000)\n\n");
	fprintf(stderr, "\t /jobmemorylimit <Set the limit for the virtual memory that can be committed for the job, in bytes> (default: 5243000000)\n\n");
	fprintf(stderr, "\t /jobprocesspeak <Set the peak memory used by any process ever associated with the job, in bytes> (default: NULL)\n\n");
	fprintf(stderr, "\t /jobprocessespeak <Set the peak memory usage of all processes currently associated with the job, in bytes> (default: NULL)\n\n");
}

int parse_and_setup(int argc, char **argv)
{
	if(argc == 1 || strcmp(argv[1], "?")==0)
	{
		return 1;
	}

	int FILE_NAME_LENGTH = strlen(argv[1]); 
	for(int i=0; i<FILE_NAME_LENGTH; i++)
	{
		if( (argv[1][i] == '\\') ||
			(argv[1][i] == '/') ||
			(argv[1][i] == ':') ||
			(argv[1][i] == '*') ||
			(argv[1][i] == '?') ||
			(argv[1][i] == '"') ||
			(argv[1][i] == '<') ||
			(argv[1][i] == '>') ||
			(argv[1][i] == '||') )
		{
			fprintf(stderr, "\n");
			fprintf(stderr, "Invalid filename: %s \n", argv[1]);
			return 1;
		}
	}

	if(argc > 1)
	{
		for(int i=2; i<argc; i++)
		{
			if(strcmp(argv[i], "/processtime") == 0)
			{
				if( (i+1) > (argc-1) )
				{
					fprintf(stderr, "The value for options %s was not specified.\n", argv[i]);
					return 1;
				}
				LARGE_INTEGER temp =  { atoi(argv[i+1]) };
				PerProcessUserTimeLimitValue = temp;				
				i+=1;
			}
			else
			if(strcmp(argv[i], "/jobtime") == 0)
			{
				if( (i+1) > (argc-1) )
				{
					fprintf(stderr, "The value for options %s was not specified.\n", argv[i]);
					return 1;
				}
				LARGE_INTEGER temp = { atoi(argv[i+1]) };
				PerJobUserTimeLimitValue= temp;
				i+=1;
			}
			else
			if(strcmp(argv[i], "/processmemorylimit") == 0)
			{
				if( (i+1) > (argc-1) )
				{
					fprintf(stderr, "The value for options %s was not specified.\n", argv[i]);
					return 1;
				}
				ProcessMemoryLimitValue = atoi(argv[i+1]);
				i+=1;
			}
			else
			if(strcmp(argv[i], "/jobmemorylimit") == 0)
			{
				if( (i+1) > (argc-1) )
				{
					fprintf(stderr, "The value for options %s was not specified.\n", argv[i]);
					return 1;
				}
				JobMemoryLimitValue = atoi(argv[i+1]);
				i+=1;
			}
			else
			if(strcmp(argv[i], "/jobprocesspeak") == 0)
			{
				if( (i+1) > (argc-1) )
				{
					fprintf(stderr, "The value for options %s was not specified.\n", argv[i]);
					return 1;
				}
				PeakProcessMemoryUsedValue = atoi(argv[i+1]);
				i+=1;
			}
			else
			if(strcmp(argv[i], "/jobprocessespeak") == 0)
			{
				if( (i+1) > (argc-1) )
				{
					fprintf(stderr, "The value for options %s was not specified.\n", argv[i]);
					return 1;
				}
				PeakJobMemoryUsedValue = atoi(argv[i+1]);
				i+=1;
			}
			else
			{
				printf("Invalid argument: %s\n", argv[i]);
				return 1;
			}
		}
	}
	return 0;
}

void test_parsing()
{
	printf("%d\n",PerProcessUserTimeLimitValue);
	printf("%d\n",PerJobUserTimeLimitValue);
	printf("%d\n",ProcessMemoryLimitValue);
	printf("%d\n",JobMemoryLimitValue);
	printf("%d\n",PeakProcessMemoryUsedValue);
	printf("%d\n",PeakJobMemoryUsedValue);
}

int safe_execute(int argc, char **argv) 
{
	SetErrorMode(SEM_FAILCRITICALERRORS);

	// Parsing arguments.
	if(parse_and_setup(argc, argv) == 1)
	{
		usage(argc, argv);
		return 1;
	}

	HANDLE job = CreateJobObject(NULL, L"Job");

	if(job == NULL){
		fprintf(stderr, "Failed to create the job object. Error: (%d).\n", GetLastError());
		return 1;
	}
	fprintf(stderr, "The Job object was created successfully.\n");

	JOBOBJECT_BASIC_LIMIT_INFORMATION job_object_basic_limit_information_struct = { 0 };
	job_object_basic_limit_information_struct.PerProcessUserTimeLimit = PerProcessUserTimeLimitValue;
	job_object_basic_limit_information_struct.PerJobUserTimeLimit = PerJobUserTimeLimitValue;
	job_object_basic_limit_information_struct.LimitFlags = LimitFlagsValue;
	job_object_basic_limit_information_struct.MinimumWorkingSetSize = MinimumWorkingSetSizeValue;
	job_object_basic_limit_information_struct.MaximumWorkingSetSize = MaximumWorkingSetSizeValue;
	job_object_basic_limit_information_struct.ActiveProcessLimit = ActiveProcessLimitValue;
	job_object_basic_limit_information_struct.Affinity = AffinityValue;
	job_object_basic_limit_information_struct.PriorityClass = PriorityClassValue;
	job_object_basic_limit_information_struct.SchedulingClass = SchedulingClassValue;

	JOBOBJECT_EXTENDED_LIMIT_INFORMATION job_object_extended_limit_information_struct = { 0 };
	job_object_extended_limit_information_struct.BasicLimitInformation = job_object_basic_limit_information_struct;
	job_object_extended_limit_information_struct.ProcessMemoryLimit = ProcessMemoryLimitValue;
	job_object_extended_limit_information_struct.JobMemoryLimit = JobMemoryLimitValue;
	job_object_extended_limit_information_struct.PeakProcessMemoryUsed = PeakProcessMemoryUsedValue;
	job_object_extended_limit_information_struct.PeakJobMemoryUsed = PeakJobMemoryUsedValue;

	if(SetInformationJobObject(job, JobObjectExtendedLimitInformation, &job_object_extended_limit_information_struct, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION)) == 0)
	{
		fprintf(stderr, "Failed to configure the job object with the information from the structures. Error: (%d).\n", GetLastError());
		return 1;
	}
	fprintf(stderr, "The job object was successfully configured with the information from the structures.\n");
	
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	HANDLE out;

	si.hStdError = hStdErr;
	si.hStdInput = hStdIn;
	si.hStdOutput = hStdOut;
	si.wShowWindow = SW_HIDE;

	_SECURITY_ATTRIBUTES sa = { 0 };
	sa.nLength = sizeof(_SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	ZeroMemory( &pi, sizeof(pi) );

	size_t converted_chars = 0;
	size_t original_size = strlen(argv[1])+1;
	wchar_t * final_string = new wchar_t[original_size];

	mbstowcs_s(&converted_chars, final_string, original_size, argv[1], _TRUNCATE);

	LPWSTR file_name = final_string;

	// Start the child process. 
	if( !CreateProcess( file_name,                      // No module name (use command line)
		NULL,                                           // Command line
		&sa,                                           // Process handle not inheritable
		&sa,                                           // Thread handle not inheritable
		TRUE,                                          // Set handle inheritance to FALSE
		CREATE_BREAKAWAY_FROM_JOB| CREATE_SUSPENDED | CREATE_NEW_CONSOLE,    // No creation flags
		NULL,                                           // Use parent's environment block
		NULL,                                           // Use parent's starting directory 
		&si,                                            // Pointer to STARTUPINFO structure
		&pi )                                           // Pointer to PROCESS_INFORMATION structure
		) 
	{
		fprintf(stderr, "Failed to create the process. Error: (%d).\n", GetLastError());
		return 1;
	}
	fprintf(stderr, "The process has started successfully.\n");

	if(AssignProcessToJobObject(job, pi.hProcess) == 0)
	{
		fprintf(stderr, "Failed to assign the process to the job object. Error: (%d).\n", GetLastError());
		return 1;
	}
	fprintf(stderr, "The process was successfully assigned.\n");


	ResumeThread(pi.hThread);


	// Wait until child process exits.
	WaitForSingleObject( pi.hProcess, INFINITE );

	JOBOBJECT_BASIC_ACCOUNTING_INFORMATION jbai = {0};

	// Close process and thread handles. 
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
	


	if(QueryInformationJobObject(job, JobObjectBasicAccountingInformation, &jbai, sizeof(JOBOBJECT_BASIC_ACCOUNTING_INFORMATION), NULL) == 0){
		fprintf(stderr, "Failed to query information from the job object. Error: (%d).\n", GetLastError());
		return 1;
	}

	CloseHandle( job );

	fprintf(stderr, "The query executed successfully.\n\n");
	fprintf(stderr, "Execution report:\n");
	fprintf(stderr, "-----------------\n\n");

	fprintf(stderr, "Total user time: --------------------------------- %lld\n", jbai.TotalUserTime.QuadPart);
	fprintf(stderr, "Total kernel time: ------------------------------- %lld\n", jbai.TotalKernelTime.QuadPart);
	fprintf(stderr, "This period total user time: --------------------- %lld\n", jbai.ThisPeriodTotalUserTime.QuadPart);
	fprintf(stderr, "This period total kernel time: ------------------- %lld\n", jbai.ThisPeriodTotalKernelTime.QuadPart);
	fprintf(stderr, "Total page fault count: -------------------------- %ld\n", jbai.TotalPageFaultCount);
	fprintf(stderr, "Total processes: --------------------------------- %ld\n", jbai.TotalProcesses);
	fprintf(stderr, "Active processes: -------------------------------- %ld\n", jbai.ActiveProcesses);
	fprintf(stderr, "Total terminated processes violating the limit: --  %ld\n", jbai.TotalTerminatedProcesses);

	return 0;
}

#endif