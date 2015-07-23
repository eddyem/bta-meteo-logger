#define		_LANG(_var, _ru, _en)	char _var##ru[] = _ru;\
					char _var##en[] = _en;\
					char *_var[2] = {_var##ru,  _var##en};
#define		_L(x)	(x[Lang])

_LANG(_s_Save_file_, "Сохранить файл", "Save file");
_LANG(_s_no_QS_, "Отсутствует строка запроса", "No query string");
_LANG(_s_no_tstart_, "Пожалуйста, заполните форму\"начальное время\"", "Please, fill the form \"starting time\"");
_LANG(_s_Date_, "Дата и время", "Date and time");
_LANG(_s_Otemp_, "Внешняя температура", "Outdoor temperature");
_LANG(_s_Itemp_, "Внутренняя температура", "Indoor temperature");
_LANG(_s_Mtemp_, "Температура зеркала", "Mirror temperature");
_LANG(_s_Ot_, "Внешняя", "Outdoor");
_LANG(_s_It_, "Внутренняя", "Indoor");
_LANG(_s_Mt_, "Зеркала", "Mirror");
_LANG(_s_WSpeed_, "Скорость ветра", "Wind speed");
_LANG(_s_Temp_, "Температура", "Temperature");
_LANG(_s_Pressure_, "Давление (мм.рт.ст.)", "Pressure (mmHg)");
_LANG(_s_Humidity_, "Влажность (%)", "Humidity (%)");
_LANG(_s_State_, "Состояние телескопа", "Telescope state");
_LANG(_s_Monlen_, "Сделано измерений", "Number of measurements");
_LANG(_s_SVGerr_, "Не могу создать график", "Can't create graph");
_LANG(_s_Bad_date_, "Неправильный формат даты, или дата окончания раньше даты начала", "Bad date format, or starting date goes after ending date");
_LANG(_s_Cant_open_cache_, "Не могу открыть файл кэша", "Can't open cache file");
_LANG(_s_Cant_open_data_, "Не могу открыть файл с данными", "Can't open data file");
_LANG(_s_Cant_write_, "Ошибка записи данных", "Can't write data");
_LANG(_s_noData_, "За указанный период нет данных", "No data for this period");
_LANG(_s_Vopen_, "Забрало открыто", "Visor is open");
_LANG(_s_Vclose_, "Забрало закрыто", "Visor is close");
_LANG(_s_CurVstat_, "Текущее состояние", "Current status");
_LANG(_s_ChVstat_, "Сменить состояние на", "Change status to");
_LANG(_s_noVisor_, "Не могу определить состояние забрала", "Can't define visor state");
_LANG(_s_No_Data_, "Недостаточно данных", "Not enough data");
_LANG(_s_Gtr_, "больше", "greater");
_LANG(_s_Less_, "меньше", "less");
_LANG(_s_G_mustbe_less_L_, "Число в поле \"больше\" должно быть меньше числа в поле \"меньше\"",
	"The value in field \"greater\" must be less than value in field \"less\"");
_LANG(_s_L_mustbe_less_G_, "Число в поле \"меньше\" должно быть меньше числа в поле \"больше\"",
	"The value in field \"less\" must be less than value in field \"greater\"");
_LANG(_s_Max_data_, "Даты с максимальным значением параметров", "Dates with maximum of paremeters");
_LANG(_s_Min_data_, "Даты с минимальным значением параметров", "Dates with minimum of paremeters");
_LANG(_s_Diapazon_, "Данные из выбранного диапазона:", "Data from selected range:");
_LANG(_s_Modes_, "Режимы работы", "Work modes");
_LANG(_s_And_, "и", "and");
_LANG(_s_Or_, "или", "or");
_LANG(_s_Mode_Times_, "Суммарное время работы в режимах", "Total work time in modes");
_LANG(_s_seconds_, "секунд", "seconds");
_LANG(_s_days_, "суток", "days");
_LANG(_s_Stopped_, "Полный останов", "Full stop");
_LANG(_s_Guiding_, "Сопровождение объекта", "Object guiding");
_LANG(_s_Ready_, "Готов", "Ready");
_LANG(_s_Other_, "Прочее", "Other");
_LANG(_s_Total_, "Итого", "Total");
//_LANG(, "", "");

