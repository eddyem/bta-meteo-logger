#define		_LANG(_var, _ru, _en)	char _var##ru[] = _ru;\
					char _var##en[] = _en;\
					char *_var[2] = {_var##ru,  _var##en};
#define		_L(x)	(x[Lang])

_LANG(_s_Save_file_, "��������� ����", "Save file");
_LANG(_s_no_QS_, "����������� ������ �������", "No query string");
_LANG(_s_no_tstart_, "����������, ��������� �����\"��������� �����\"", "Please, fill the form \"starting time\"");
_LANG(_s_Date_, "���� � �����", "Date and time");
_LANG(_s_Otemp_, "������� �����������", "Outdoor temperature");
_LANG(_s_Itemp_, "���������� �����������", "Indoor temperature");
_LANG(_s_Mtemp_, "����������� �������", "Mirror temperature");
_LANG(_s_Ot_, "�������", "Outdoor");
_LANG(_s_It_, "����������", "Indoor");
_LANG(_s_Mt_, "�������", "Mirror");
_LANG(_s_WSpeed_, "�������� �����", "Wind speed");
_LANG(_s_Temp_, "�����������", "Temperature");
_LANG(_s_Pressure_, "�������� (��.��.��.)", "Pressure (mmHg)");
_LANG(_s_Humidity_, "��������� (%)", "Humidity (%)");
_LANG(_s_State_, "��������� ���������", "Telescope state");
_LANG(_s_Monlen_, "������� ���������", "Number of measurements");
_LANG(_s_SVGerr_, "�� ���� ������� ������", "Can't create graph");
_LANG(_s_Bad_date_, "������������ ������ ����, ��� ���� ��������� ������ ���� ������", "Bad date format, or starting date goes after ending date");
_LANG(_s_Cant_open_cache_, "�� ���� ������� ���� ����", "Can't open cache file");
_LANG(_s_Cant_open_data_, "�� ���� ������� ���� � �������", "Can't open data file");
_LANG(_s_Cant_write_, "������ ������ ������", "Can't write data");
_LANG(_s_noData_, "�� ��������� ������ ��� ������", "No data for this period");
_LANG(_s_Vopen_, "������� �������", "Visor is open");
_LANG(_s_Vclose_, "������� �������", "Visor is close");
_LANG(_s_CurVstat_, "������� ���������", "Current status");
_LANG(_s_ChVstat_, "������� ��������� ��", "Change status to");
_LANG(_s_noVisor_, "�� ���� ���������� ��������� �������", "Can't define visor state");
_LANG(_s_No_Data_, "������������ ������", "Not enough data");
_LANG(_s_Gtr_, "������", "greater");
_LANG(_s_Less_, "������", "less");
_LANG(_s_G_mustbe_less_L_, "����� � ���� \"������\" ������ ���� ������ ����� � ���� \"������\"",
	"The value in field \"greater\" must be less than value in field \"less\"");
_LANG(_s_L_mustbe_less_G_, "����� � ���� \"������\" ������ ���� ������ ����� � ���� \"������\"",
	"The value in field \"less\" must be less than value in field \"greater\"");
_LANG(_s_Max_data_, "���� � ������������ ��������� ����������", "Dates with maximum of paremeters");
_LANG(_s_Min_data_, "���� � ����������� ��������� ����������", "Dates with minimum of paremeters");
_LANG(_s_Diapazon_, "������ �� ���������� ���������:", "Data from selected range:");
_LANG(_s_Modes_, "������ ������", "Work modes");
_LANG(_s_And_, "�", "and");
_LANG(_s_Or_, "���", "or");
_LANG(_s_Mode_Times_, "��������� ����� ������ � �������", "Total work time in modes");
_LANG(_s_seconds_, "������", "seconds");
_LANG(_s_days_, "�����", "days");
_LANG(_s_Stopped_, "������ �������", "Full stop");
_LANG(_s_Guiding_, "������������� �������", "Object guiding");
_LANG(_s_Ready_, "�����", "Ready");
_LANG(_s_Other_, "������", "Other");
_LANG(_s_Total_, "�����", "Total");
//_LANG(, "", "");

