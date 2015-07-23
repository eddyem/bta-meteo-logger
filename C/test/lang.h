#define		_LANG(_var, _ru, _en)	char _var##ru[] = _ru;\
					char _var##en[] = _en;\
					char *_var[2] = {_var##ru,  _var##en};
#define		_L(x)	(x[Lang])

_LANG(_s_Name_, "Ваши Ф.И.О.", "Your name");
_LANG(_s_Stop_n_write_, "Остановить запись и сохранить файл", "Stop writing and save file");
_LANG(_s_Mon_didnt_start_, "С вашего IP мониторинг не запускался", "Monitoring didn't start from your IP");
_LANG(_s_Err_, "Ошибка!", "Error!");
_LANG(_s_Cant_shmat_, "Не могу подключить область разделяемой памяти", "Can't get shared memory segment");
_LANG(_s_Try_again_, "Попробуйте еще раз позже", "Try again later");
_LANG(_s_Cant_find_IP_, "Не могу найти ваш IP-адрес в таблице", "Can't find your IP in table");
_LANG(_s_Cant_shmget_, "Не могу получить доступ к разделяемой памяти", "Can't get a memory segment");
_LANG(_s_Mon_running_, "С вашего IP мониторинг уже запущен", "Monitoring from your IP is running");
_LANG(_s_Startmon_, "Начинаю мониторинг", "Starting monitoring");
_LANG(_s_Can_close_, "Можете закрыть это окно", "You can close this window");
_LANG(_s_No_IP_, "Не могу определить ваш IP-адрес", "Can't determine your IP");
_LANG(_s_Solvethis_, "Пожалуйста, решите эту проблему и попробуйте снова", "Please, solve this problem and try again");
_LANG(_s_Fillname_, "Пожалуйста, заполните форму\"Имя\"", "Please, fill the form \"Name\"");
_LANG(_s_Refresh_, "Обновить", "Refresh");
_LANG(_s_Refreshing_, "Средние данные будут обновляться каждые 60 секунд", "Average data will refresh every 60 seconds");
_LANG(_s_Otemp_, "Внешняя", "Outdoor");
_LANG(_s_Itemp_, "Внутренняя", "Indoor");
_LANG(_s_Mtemp_, "Зеркала", "Mirror");
_LANG(_s_WSpeed_, "Скорость ветра", "Wind speed");
_LANG(_s_Temp_, "Температура", "Temperature");
_LANG(_s_Monlen_, "Сделано измерений", "Number of measurements");
_LANG(_s_SVGerr_, "Не могу создать график", "Can't create graph");
_LANG(_s_FIFOerr_, "Ошибка FIFO", "FIFO error");
//_LANG(, "", "");

