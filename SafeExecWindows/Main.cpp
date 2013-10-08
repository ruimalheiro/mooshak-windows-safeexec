#include <stdio.h>
#include <windows.h>
#include <Aclapi.h>
#include <WinError.h>

// JOBOBJECT_BASIC_LIMIT_INFORMATION structure
// More info: http://msdn.microsoft.com/en-us/library/windows/desktop/ms684147(v=vs.85).aspx

LARGE_INTEGER PerProcessUserTimeLimitValue = { 1000 };
LARGE_INTEGER PerJobUserTimeLimitValue     = { 1000 };
DWORD         LimitFlagsValue              = NULL;
SIZE_T        MinimumWorkingSetSizeValue   = NULL;
SIZE_T        MaximumWorkingSetSizeValue   = NULL;
DWORD         ActiveProcessLimitValue      = NULL;
ULONG_PTR     AffinityValue                = NULL;
DWORD         PriorityClassValue           = NULL;
DWORD         SchedulingClassValue         = NULL;



// JOBOBJECT_EXTENDED_LIMIT_INFORMATION structure
// More info: http://msdn.microsoft.com/en-us/library/windows/desktop/ms684156(v=vs.85).aspx

SIZE_T       ProcessMemoryLimitValue    = 100000000;
SIZE_T       JobMemoryLimitValue        = 100000000;
SIZE_T       PeakProcessMemoryUsedValue = NULL;
SIZE_T       PeakJobMemoryUsedValue     = NULL;



int main(int argc, char ** argv){

	SetErrorMode(SEM_FAILCRITICALERRORS);

	HANDLE job = CreateJobObject(NULL, L"Job");

	if(job == NULL){
		printf("Failed to create the job object. Error: (%d).\n", GetLastError());
		system("PAUSE");
		return 1;
	}
	printf("The Job object was created successfully.\n");

	JOBOBJECT_BASIC_LIMIT_INFORMATION job_object_basic_limit_information_struct = { 0 };
	job_object_basic_limit_information_struct.PerProcessUserTimeLimit = PerProcessUserTimeLimitValue;
	job_object_basic_limit_information_struct.PerJobUserTimeLimit = PerJobUserTimeLimitValue;
	job_object_basic_limit_information_struct.LimitFlags = JOB_OBJECT_LIMIT_PROCESS_MEMORY | 
		                                                   JOB_OBJECT_LIMIT_JOB_MEMORY | 
					                                       JOB_OBJECT_LIMIT_JOB_TIME | 
					                                       JOB_OBJECT_LIMIT_PROCESS_TIME;
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
		printf("Failed to configure the job object with the information from the structures. Error: (%d).\n", GetLastError());
		system("PAUSE");
		return 1;
	}
	printf("The job object was successfully configured with the information from the structures.\n");

	STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );


    // Start the child process. 
    if( !CreateProcess( L"bomb.bat",                    // No module name (use command line)
        NULL,                                           // Command line
        NULL,                                           // Process handle not inheritable
        NULL,                                           // Thread handle not inheritable
        FALSE,                                          // Set handle inheritance to FALSE
        CREATE_BREAKAWAY_FROM_JOB| CREATE_SUSPENDED,    // No creation flags
        NULL,                                           // Use parent's environment block
        NULL,                                           // Use parent's starting directory 
        &si,                                            // Pointer to STARTUPINFO structure
        &pi )                                           // Pointer to PROCESS_INFORMATION structure
    ) 
    {
        printf("Failed to create the process. Error: (%d).\n", GetLastError());
		system("PAUSE");
        return 1;
    }
	printf("The process has started successfully.\n");
	
	if(AssignProcessToJobObject(job, pi.hProcess) == 0)
	{
		printf("Failed to assign the process to the job object. Error: (%d).\n", GetLastError());
		system("PAUSE");
		return 1;
	}
	printf("The process was successfully assigned.\n");


	ResumeThread(pi.hThread);


    // Wait until child process exits.
    WaitForSingleObject( pi.hProcess, INFINITE );

	JOBOBJECT_BASIC_ACCOUNTING_INFORMATION jbai = {0};
	if(QueryInformationJobObject(job, JobObjectBasicAccountingInformation, &jbai, sizeof(JOBOBJECT_BASIC_ACCOUNTING_INFORMATION), NULL) == 0){
		printf("Failed to query information from the job object. Error: (%d).\n", GetLastError());
		system("PAUSE");
		return 1;
	}
	printf("The query executed successfully.\n\n");
	printf("Execution report:\n");
	printf("-----------------\n\n");

	printf("Total user time:               %d\n", jbai.TotalUserTime);
	printf("Total kernel time:             %d\n", jbai.TotalKernelTime);
	printf("This period total user time:   %d\n", jbai.ThisPeriodTotalUserTime);
	printf("This period total kernel time: %d\n", jbai.ThisPeriodTotalKernelTime);
	printf("Total page fault count:        %d\n", jbai.TotalPageFaultCount);
	printf("Total processes:               %d\n", jbai.TotalProcesses);
	printf("Active processes:              %d\n", jbai.ActiveProcesses);
	printf("Total terminated processes:    %d\n", jbai.TotalTerminatedProcesses);

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

	system("PAUSE");

	return 0;
}