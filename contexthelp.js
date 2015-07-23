var tipobj=null;

function startHelp(evt){
	evt.stopPropagation();
	evt.preventDefault();
	document.body.style.cursor = "help";
	document.body.onclick = Help;
	document.body.onmouseover = stoponclick;
}

function Help(evt){
	evt.stopPropagation();
	evt.preventDefault();
	if(!helptip(evt)) return;
	document.body.onclick = '';
	document.body.style.cursor = "default";
	document.body.onmouseover = '';
}

var oldclc, oldmout;
function stoponclick(evt){
	var obj = evt.target;
	if(obj == document.body) return;
	oldclc = obj.onclick;
	oldmout = obj.onmouseout;
	obj.onclick = Help;
	obj.onmouseout = releaseonclick;
	obj.style.cursor = "help";
}
function releaseonclick(evt){
	var obj = evt.target;
	obj.onmouseout = oldmout;
	obj.onclick = oldclc;
	obj.style.cursor = "default";
}
function helptip(evt){
	var ss = helpgen(evt), helper;
	if(ss.length > 0){
		tipobj = document.createElement("DIV");
		tipobj.id = 'helptip';
		tipobj.setAttribute("name", "helptip");
		tipobj.onclick = rmtip;
		tipobj.innerHTML = ss;
		helper = document.createElement("DIV");
		helper.className = 'redtxt';
		helper.innerHTML = "����� ������� ��� ����, �������� �� ���� ����� ������� ���� ��� ������� ������� ESCAPE";
		helper.onclick = function(evt){evt.stopPropagation(); 
			document.body.removeChild(evt.target.parentNode);};
		tipobj.appendChild(helper);
		document.body.appendChild(tipobj);
		positiontip(evt);
	}
	releaseonclick(evt);
	return (ss.length);
}

function helpgen(evt){
	var obj = evt.target;
	var objid, objname, ss="", nm;
	onkey(27);
	objid = obj.id; objname = obj.name;
	if(objid == "" && objname == null){
		objid = obj.parentNode.id;
		objname = obj.parentNode.name;
	}
	if(objid == "" && objname == null) return (ss);
	if(objid == "" && objname != null) nm = objname;
	else nm = objid;
	ss = HelpText[nm];
	if(ss == null) ss = HelpText[obj.parentNode.id];
	return (ss);
}

function positiontip(e){
	var wd = tipobj.offsetWidth, ht = tipobj.offsetHeight;
	var curX = e.clientX + 25;
	var curY = e.clientY - ht/2;
	var btmedge = document.body.clientHeight - curY - 15;
	var rightedge = document.body.clientWidth - curX - 15;
	if(rightedge < wd) curX -= wd+50;
	if(btmedge < ht) curY -= ht-btmedge+15;
	if(curY < 15) curY = 15;
	tipobj.style.left = curX+"px";
	tipobj.style.top = curY+"px";
}

function onkey(code){
	if(code != 27) return;
	var helps = document.getElementsByName('helptip');
	var l = helps.length-1; 
	for(var i=l; i>-1; i--) document.body.removeChild(helps[i]);
}
function rmtip(evt){
	document.body.removeChild(evt.target);
}

const HelpText = {
	dtheader: "���� ����������� ����, � ������� ���������� ������ ��������� � (�����������) �������� �������� ��������� �������, �� �������� ����� ������������� ����� ������.<p></p>�� ������ ��������������� ������� ���������� (������� �� ������ ������ �� ����� ����� ���� � �������) ��� �� ������ ������ �������.<p></p>��� ����� ������ ������� �������� �������� �� ������ ���� � �������: ���� �������� � ������� �����/�����/���, ����� ����� ������ �������� ����� � ������� ����:������. ��� ���������� ������ ������ (�.�. 2010, � �� ������ 10).<p></p>��������, ����� \"7 ����� 35 ����� 10 ����� 2010 ����\" ���������� ������� ���:<p></p>\"03/10/2010 7:35\".",
	t_beg: "������� � ��� ���� �����, ������� � �������� �� ������ �������� �������������� ������.",
	t_end: "�������� � ��� ���� ����� ��������� ���������.<p></p>���� �� �������� ���� ������, �������� ����� ��������� ������� �����.",
	cal: "������� ����, ���� �� ������ ������� ����� � ���� ��� ������ �������� ���������.<p></p>� ����� ��������� ������� ������� ������ �����, ����� �������� ������ ��� � ����� � �������� �� ������ ����.",
	reqbtn: "����� ����� ���������� � (�����������) ��������� �������� ������������� ��� ��������� �������, ������� �� ��� ������ ��� ���������� �������� � ��������� �������������� ����������.",
	avertime: "��� ���� ��������� ������� �������� �������, �� �������� ����� ������������� ��������� ���������� ������, ���������� ���� ��� ������� �� ������ \"��������� ����\".",
	averinterval: "������� � ��� ���� ���������� �������, �� �������� ����� ������������� ��������� ����������.",
	averval: "�������� ������� ��������� �������: ������, ���� ��� �����.",
	modes: "�������� ������������ ������ ������ ���������.<p></p>��� ������ �� ���� ������� ����� ���� ������� �������������.",
	Mstp: "������� ��������� � ������ ���������.",
	Mgd: "����� ������������� ������� (������ �����, � ��� ����� ����������� ����������).",
	Mrd: "�������� ����� �������: �������� ����� � ������.",
	Moth: "�����, �� ���������� � ���������� (��������, ��������� �� ������).",
	Mopn: "�� ��������� ���������� ����������.<p></p>��������! ���� ����� �� ��������������� ������������� � ������� �� ����, �� ������� �� �������� ��� ������ ���������� ������ ������ \"������� �������\", � ����� �� ��������� - ������ \"������� �������\".",
	Statbtn: "��� ������� ���� ������ ������������/���������� ����� ��� ������� �������������� ����������: ������������� �������� ���������������, ������������ �������� ���������������.",
	Visorbtn: "��� ������� �� ��� ������ ������������ ����� \"������� �������/�������\".<p></p>������� ������ ������������ ��������� �������.",
	StMax: "������� �� ��� ������ ��� ����������� ������������ �������� ���� ��������������� �� ��������� ���������� �������.",
	StMin: "������� �� ��� ������ ��� ����������� ����������� �������� ���� ��������������� �� ��������� ���������� �������.",
	StMode: "������� �� ��� ������, ���� ������ ������, ������� ������� �������� ������ � ������ �� ������� �� ��������� ���������� �������.",
	morethan: "������� � ��� ���� ������ ������� �������� ������������� ��������������.<p></p>�������� ������ ���� ������� \"��������\" �� ����� \"����������\".",
	and_or: "����� ������, ��������� ��������: \"�\" � \"���\". �������:<ul><li>��� ������ �������� ������ ��������� ��������� ���� \"������\", �������� ������ ���� \"������\", �������� ������ \"���\";</li><li>��� ������ �������� ������ ��������� �������� ������������� ���� \"������\", ��������� ���� \"������\", ������ - \"���\";</li><li>���� �� �������� ������ \"���\" � ��������� ��� ����, ����� ������� �������� <b>���</b> ��������� ����� \"������\" � \"������\", ����� � ���� \"������\" ������ ���� ������ ����� � ���� \"������\";</li><li>��� ������ ������ \"�\" ��� ���� ������ ���� ����������� ���������, ������������ ������ <b>������</b> ��������� ����� \"������\" � \"������\", ����� � ���� \"������\" ������ ���� ������ ����� � ���� \"������\".</li></ul>���� � ���������� ������� ������� ���������� �� ������������, ������, ��������� �������������� ����� �������� ������ ��������� ��������� � ������� ����� ���������� �������.",
	lessthan: "������� � ��� ���� ������� ������� �������� ������������� ��������������.<p></p>�������� ������ ���� ������� \"��������\" �� ����� \"����������\".",
	MkStat: "����� ���������� ����� �����, ������� ��� ������ ��� ��������� ����������.",
	Sall: "�������� �� ��������������, ������� �� ������ ������ �� �������, ��� �� ������� �� ������ ��������� ��������� ������� � ������������� ����������.<p></p>��� ������ �� ���� ������� ������������� ����� ���� ���������������.",
	Stemp: "�������� �� ���� ������� ��� �������������� ������ ����������.",	
	Sout: "���������� ������ �� ������� �����������.",
	Sin: "���������� ������ �� ���������� �����������.",
	Smr: "���������� ������ �� ����������� �������.",
	Swnd: "���������� ������ �� �������� �����.",
	Sprs: "���������� ������ �� ������������ ��������.",
	Shmd: "���������� ������ �� ������������� ���������.",
	imcontrols: "�������� ������ ����������� ��� ����������� ��������.",
	SVG: "��������� ������ SVG ����� ����� �������� ��� ����������� ��������, ����� ���� �������� � ����� ����������� ��� ����� (�� ������ ������ ������������� ��� � eps ��� ������ ������� convert ������ ImageMagic).<p><p>��������! ������ �������� �� �������� ���������� ���� ������. ���� ��� ������� �� ����� ���������� �������, ���� �������� ���, ���� �������� ������ ������ �����������.",
	JPEG: "�������� ���������� ������ ��� �������� ��������� �����������. ����� ��������� ��������, � ��� �� ������ ���������.",
	GIF: "��������� ������, �������� ����� JPEG.<p></p>JPEG � GIF �������� ��� ��������� � ����������� ��� ������, �.�. ����� �������������� (��-�� ����� ��������� �������)."
}
