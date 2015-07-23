var request;
var timeout_id, monit_tid;
var CGI_PATH = '/cgi-bin/eddy/tempmon';
var ShowOnGraph = 0, ImType = '1';
var isRunning = 0, t_start = 0, t_end = 0, Stat = 0;
function $(Id){
	return document.getElementById(Id);
}

var utf2koi={
1025:179,
1072:193,
1073:194,
1074:215,
1075:199,
1076:196,
1077:197,
1078:214,
1079:218,
1080:201,
1081:202,
1082:203,
1083:204,
1084:205,
1085:206,
1086:207,
1087:208,
1088:210,
1089:211,
1090:212,
1091:213,
1092:198,
1093:200,
1094:195,
1095:222,
1096:219,
1097:221,
1098:223,
1099:217,
1100:216,
1101:220,
1102:192,
1103:209,
1105:163
};

function hexdump(str){
	var ret = "";
	var l = str.length;
	var i, ch, code;
	for(i=0; i<l; i++){
		ch = str.charCodeAt(i);
		if(utf2koi[ch] != null)
			code = utf2koi[ch];
		else if(utf2koi[ch+32] != null)
			code = utf2koi[ch+32]+32;
		else code = ch;
		ch = code.toString(16);
		ret += "%" + ch;
	}
	return ret;
}

function TimeOut(){
	request.abort();
	handleError("Time over");
}

function handleError(msg) {
	$('body').innerHTML = "<h1>Ошибка xmlhttprequest:<br>" + msg + "</h1>";
}

function ch_status(){
	$('body').innerHTML = request.responseText;
	var scripts = $('body').getElementsByTagName("script");
	for(var i=0,len=scripts.length; i<len; i++)
		eval.call(window, scripts[i].innerHTML);
}
function sendrequest(req_STR, fn_OK){
	request = new XMLHttpRequest();
	request.open("POST", CGI_PATH, true);
	request.setRequestHeader("Accept-Charset", "koi8-r");
	request.overrideMimeType("multipart/form-data; charset=koi8-r"); 
	request.onreadystatechange=function(){
		if (request.readyState == 4){
			if (request.status == 200){
				clearTimeout(timeout_id);
				fn_OK();
			}
			else handleError(request.statusText);
		}
	}
	request.send(req_STR);
	timeout_id = setTimeout(TimeOut, 10000);
}

function chkvals(){
	var i;
	var Boxes = document.getElementsByName('show');
	ShowOnGraph = 0;
	for(i = 0; i < Boxes.length; i++)
		if(Boxes[i].checked) ShowOnGraph += parseInt(Boxes[i].value);
	var Type = document.getElementsByName('type');
	for(i = 0; i < Type.length; i++)
		if(Type[i].checked){
			ImType = Type[i].value;
			break;
		}
	var mask = 0;
	Boxes = document.getElementsByName('mask');
	for(i = 0; i < Boxes.length; i++)
		if(Boxes[i].checked) mask += parseInt(Boxes[i].value);
	if(isRunning && !Stat)
		mkImage(mask);
	return mask;
}

function checkAll(ii, name){
	if(!name) name = 'show';
	var Boxes = document.getElementsByName(name);
	var i;
	if(!ii) ii = Boxes.length;
	for(i = 0; i < ii; i++)
		Boxes[i].checked = !Boxes[i].checked;
	chkvals();
}

function get_seconds(blkid){
	var td_value = $(blkid).value;
	var seconds = Math.round(Date.parse(td_value) / 1000);
	return seconds;
}

function clear_cencol(){
	var cencol = $('cencol');
	if(!cencol.childNodes) return;
	while(cencol.childNodes.length > 0)
		cencol.removeChild(cencol.lastChild);
}

function mkImage(mask){
	if(ShowOnGraph == 0) return;
	clear_cencol();
	if(!mask) return;
	if(ImType == '0')
		var image_ = document.createElement('embed');
	else
		var image_ = document.createElement('img');
	image_.id = 'IMG';
	$('cencol').appendChild(image_);
	var str = CGI_PATH + '?Graph=' + ShowOnGraph + '&' + 'Gtype=' + ImType;
	if(t_start) str += '&Tstart=' + t_start;
	else return;
	if(t_end) str += '&Tend=' + t_end;
	str += '&Stat=' + mask;
	$('IMG').src = str + '?' + Math.random();
}

function make_request(stat_type){
	if(!$('t_beg').value){
		alert("Введите начальное время");
		return;
	}
	var averval = "";
	if(Stat){
		var value = parseInt($('averinterval').value);
		if(!value || value < 1){
			alert("Введите правильный интервал усреднения");
			return;
		}
		var mul = parseInt($('averval').value);
		value *= mul;
		averval = ' Aver=' + value + ' ';
	}
	var str1 = averval;
	if(typeof(stat_type) != "undefined"){
		averval += ' Select='+stat_type+' ';
		if(stat_type == 3){
			var morethan = parseFloat($('morethan').value);
			var lessthan = parseFloat($('lessthan').value);
			var and_or = parseInt($('and_or').value);
			if(!morethan && !lessthan){
				alert("Не заполнены поля");
				return;
			}
			if(and_or){
				if(!morethan || !lessthan){
					alert("Заполните оба поля (\"больше\" и \"меньше\")");
					return;
				}
				if(morethan >= lessthan){
					alert("Величина в поле \"больше\" должна быть меньше величины в поле \"меньше\"\n"+
						"Либо измените логику на \"ИЛИ\"");
					return;
				}
			}
			else{
					if(morethan <= lessthan){
					alert("Величина в поле \"больше\" должна быть больше величины в поле \"меньше\"\n"+
						"Либо измените логику на \"И\"");
					return;
				}			
			}
			if(morethan) averval += ' Greater='+morethan+' ';
			if(lessthan) averval += ' Less='+lessthan+' ';
			averval += ' AndOr='+and_or+' ';
		}
		averval += ' Graph=' + ShowOnGraph + ' ';
	}
	var td = new Date();
	var fd = new Date();
	var t_now = Math.round(td / 1000);
	t_start = get_seconds('t_beg');
	if(t_start >= t_now){
		alert("Начальное время в будущем :)");
		return;
	}
	var str = 'Tstart=' + t_start;
	if($('t_end').value){
		t_end = get_seconds('t_end');
		str += ' Tend=' + t_end;
		td.setTime(t_end * 1000);}
	else t_end = 0;
	isRunning = 1;
	var mask = chkvals();
	str += ' Stat=' + mask;
	str1 += str;
	str += averval;
	function getstat(){$('Stat').innerHTML = request.responseText;
		if(mask) sendrequest(str1, ch_status);}
	function ok(){visor(); if(typeof(stat_type) != "undefined") sendrequest(str, getstat);
		else if(mask) sendrequest(str1, ch_status);}
	sendrequest("Visor=-1", ok);
	fd.setTime(t_start * 1000);
	$('header').innerHTML = 'Данные с ' + fd.toLocaleFormat("%H:%M %d/%m/%Y") +
		' по ' + td.toLocaleFormat("%H:%M %d/%m/%Y");
}

function visor(){
	$('visorbtn').innerHTML = request.responseText;
}

function init(){
	sendrequest("Visor=-1", visor);
}

function Show(id){
	$(id).style.display = 'block';
}

function Hide(id){
	$(id).style.display = 'none';
}

function statistics(){
	if(Stat){
		Stat = 0;
		clear_cencol();
		$('Statbtn').value = 'Показать статистику';
		Show('imcontrols');
		Hide('avertime');
		chkvals();
	}
	else{
		Stat = 1;
		clear_cencol();
		$('Statbtn').value = "Убрать статистику";
		var stat_div = document.createElement('div');
		stat_div.id = 'Stat';
		var stat_btns = document.createElement('div');
		stat_btns.className = 'C';
		stat_btns.innerHTML = "<h3>Отобразить интервалы времени с характеристиками:</h3><p>\n"+
			"<input type='button' OnClick='make_request(1);' value='Максимум' id='StMax'>&nbsp;&nbsp;&nbsp;\n"+
			"<input type='button' OnClick='make_request(2);' value='Минимум' id='StMin'>&nbsp;&nbsp;&nbsp;\n"+
			"<input type='button' OnClick='make_request(0);' value='Режим работы' id='StMode'><p>"+
			"В диапазоне: больше <input id='morethan' size=8> "+
			"<select id='and_or'><option value='1'>И<option value='0'>ИЛИ</select>"+
			" меньше <input id='lessthan' size=8> "+
			"<input type='button' OnClick='make_request(3);' value='OK' id='MkStat'><p>\n";
		$('cencol').appendChild(stat_btns);
		$('cencol').appendChild(stat_div);
		Hide('imcontrols');
		Show('avertime');
	}
}

