const char* msg    = OBF("kernel32.dll");
const char* safer  = OBF_AUTO("CreateRemoteThread");   // -> recommended

const wchar_t* wmsg = OBF_W(L"ntdll.dll!NtCreateSection");
