#define		_LANG(_var, _ru, _en)	char _var##ru[] = _ru;\
					char _var##en[] = _en;\
					char *_var[2] = {_var##ru,  _var##en};
#define		_L(x)	(x[Lang])

_LANG(_s_Name_, "���� �.�.�.", "Your name");
_LANG(_s_Stop_n_write_, "���������� ������ � ��������� ����", "Stop writing and save file");
_LANG(_s_Mon_didnt_start_, "� ������ IP ���������� �� ����������", "Monitoring didn't start from your IP");
_LANG(_s_Err_, "������!", "Error!");
_LANG(_s_Cant_shmat_, "�� ���� ���������� ������� ����������� ������", "Can't get shared memory segment");
_LANG(_s_Try_again_, "���������� ��� ��� �����", "Try again later");
_LANG(_s_Cant_find_IP_, "�� ���� ����� ��� IP-����� � �������", "Can't find your IP in table");
_LANG(_s_Cant_shmget_, "�� ���� �������� ������ � ����������� ������", "Can't get a memory segment");
_LANG(_s_Mon_running_, "� ������ IP ���������� ��� �������", "Monitoring from your IP is running");
_LANG(_s_Startmon_, "������� ����������", "Starting monitoring");
_LANG(_s_Can_close_, "������ ������� ��� ����", "You can close this window");
_LANG(_s_No_IP_, "�� ���� ���������� ��� IP-�����", "Can't determine your IP");
_LANG(_s_Solvethis_, "����������, ������ ��� �������� � ���������� �����", "Please, solve this problem and try again");
_LANG(_s_Fillname_, "����������, ��������� �����\"���\"", "Please, fill the form \"Name\"");
_LANG(_s_Refresh_, "��������", "Refresh");
_LANG(_s_Refreshing_, "������� ������ ����� ����������� ������ 60 ������", "Average data will refresh every 60 seconds");
_LANG(_s_Otemp_, "�������", "Outdoor");
_LANG(_s_Itemp_, "����������", "Indoor");
_LANG(_s_Mtemp_, "�������", "Mirror");
_LANG(_s_WSpeed_, "�������� �����", "Wind speed");
_LANG(_s_Temp_, "�����������", "Temperature");
_LANG(_s_Monlen_, "������� ���������", "Number of measurements");
_LANG(_s_SVGerr_, "�� ���� ������� ������", "Can't create graph");
_LANG(_s_FIFOerr_, "������ FIFO", "FIFO error");
//_LANG(, "", "");

