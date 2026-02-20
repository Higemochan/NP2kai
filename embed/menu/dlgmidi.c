#include	<compiler.h>
#include	<common/strres.h>
#include	"np2.h"
#include	<sysmng.h>
#include	<pccore.h>
#include	<embed/vramhdl.h>
#include	<embed/menubase/menubase.h>
#include	<embed/menu/menustr.h>
#include	"sysmenu.res"
#include	<embed/menu/dlgmidi.h>


enum {
	DID_MPU_EN	= DID_USER,
	DID_PORT,
	DID_PORTSTR,
	DID_INT0,
	DID_INT1,
	DID_INT2,
	DID_INT5,
	DID_OUT_VERMOUTH,
	DID_OUT_MIDIDEV,
	DID_MDL,
	DID_MDLSTR
};

static const OEMCHAR str_mpupc98[] = OEMTEXT("MPU-PC98");
static const OEMCHAR str_enable[] = OEMTEXT("MPU-98II Enable");
static const OEMCHAR str_ioport[] = OEMTEXT("I/O Port");
static const OEMCHAR str_port[] = OEMTEXT("Port");
static const OEMCHAR str_interrupt[] = OEMTEXT("Interrupt");
static const OEMCHAR str_int0[] = OEMTEXT("INT0");
static const OEMCHAR str_int1[] = OEMTEXT("INT1");
static const OEMCHAR str_int2[] = OEMTEXT("INT2");
static const OEMCHAR str_int5[] = OEMTEXT("INT5");
static const OEMCHAR str_midiout[] = OEMTEXT("MIDI Output");
static const OEMCHAR str_vermouth[] = OEMTEXT("VERMOUTH");
static const OEMCHAR str_mididev[] = OEMTEXT("MIDI Device");
static const OEMCHAR str_module[] = OEMTEXT("Module");

static const OEMCHAR *str_mdlnames[] = {
	OEMTEXT("MT-32"),	OEMTEXT("CM-32L"),	OEMTEXT("CM-64"),
	OEMTEXT("CM-300"),	OEMTEXT("CM-500LA"),	OEMTEXT("CM-500GS"),
	OEMTEXT("SC-55"),	OEMTEXT("SC-88"),	OEMTEXT("LA"),
	OEMTEXT("GM"),		OEMTEXT("GS"),		OEMTEXT("XG")
};

#define	MIDI_MDLCOUNT	(sizeof(str_mdlnames) / sizeof(str_mdlnames[0]))

static const MENUPRM res_midi[] = {
			{DLGTYPE_CHECK,		DID_MPU_EN,		MENU_TABSTOP,
				str_enable,								  8,   8, 200,  13},
			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_ioport,								  8,  28, 200,  55},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_port,								 20,  45,  36,  13},
			{DLGTYPE_SLIDER,	DID_PORT,		MSS_BOTH | MENU_TABSTOP,
				(void *)SLIDERPOS(0, 15),				 60,  45, 100,  13},
			{DLGTYPE_RTEXT,		DID_PORTSTR,	0,
				NULL,									164,  45,  36,  13},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_interrupt,							 20,  63,  36,  13},
			{DLGTYPE_RADIO,		DID_INT0,		MENU_TABSTOP,
				str_int0,								 60,  63,  40,  13},
			{DLGTYPE_RADIO,		DID_INT1,		0,
				str_int1,								100,  63,  40,  13},
			{DLGTYPE_RADIO,		DID_INT2,		0,
				str_int2,								140,  63,  40,  13},
			{DLGTYPE_RADIO,		DID_INT5,		0,
				str_int5,								180,  63,  40,  13},
			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_midiout,							  8,  90, 200,  35},
			{DLGTYPE_RADIO,		DID_OUT_VERMOUTH,	MENU_TABSTOP,
				str_vermouth,							 20, 108,  88,  13},
			{DLGTYPE_RADIO,		DID_OUT_MIDIDEV,	0,
				str_mididev,							110, 108,  88,  13},
			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_module,								  8, 132, 200,  38},
			{DLGTYPE_SLIDER,	DID_MDL,		MSS_BOTH | MENU_TABSTOP,
				(void *)SLIDERPOS(0, 11),				 20, 148, 120,  13},
			{DLGTYPE_RTEXT,		DID_MDLSTR,		0,
				NULL,									144, 148,  56,  13},
			{DLGTYPE_BUTTON,	DID_OK,			MENU_TABSTOP,
				mstr_ok,								218,  13,  77,  21},
			{DLGTYPE_BUTTON,	DID_CANCEL,		MENU_TABSTOP,
				mstr_cancel,							218,  38,  77,  21}};


// ----

static const OEMCHAR str_portfmt[] = OEMTEXT("%04X");

static void setportstr(void) {

	UINT	port;
	OEMCHAR	work[8];

	port = menudlg_getval(DID_PORT);
	OEMSNPRINTF(work, sizeof(work), str_portfmt, 0xC0D0 + (port << 10));
	menudlg_settext(DID_PORTSTR, work);
}

static void setmdlstr(void) {

	UINT	mdl;

	mdl = menudlg_getval(DID_MDL);
	if (mdl >= MIDI_MDLCOUNT) {
		mdl = MIDI_MDLCOUNT - 1;
	}
	menudlg_settext(DID_MDLSTR, str_mdlnames[mdl]);
}

static UINT module2index(const char *mdl) {

	UINT	i;

	for (i = 0; i < MIDI_MDLCOUNT; i++) {
		if (!milstr_cmp(mdl, str_mdlnames[i])) {
			return i;
		}
	}
	return 9;	/* default: GM */
}

static void dlginit(void) {

	MENUID	id;
	UINT	port_idx;
	UINT	int_idx;

	menudlg_appends(res_midi, NELEMENTS(res_midi));

	menudlg_setval(DID_MPU_EN, np2cfg.mpuenable);

	/* I/O port: upper nibble of mpuopt */
	port_idx = (np2cfg.mpuopt >> 4) & 0x0f;
	menudlg_setval(DID_PORT, port_idx);

	/* Interrupt: lower 2 bits of mpuopt */
	int_idx = np2cfg.mpuopt & 0x03;
	switch (int_idx) {
		case 0:		id = DID_INT0;	break;
		case 1:		id = DID_INT1;	break;
		case 2:		id = DID_INT2;	break;
		case 3:		id = DID_INT5;	break;
		default:	id = DID_INT2;	break;
	}
	menudlg_setval(id, 1);

	/* MIDI output */
	if (!milstr_cmp(np2oscfg.mpu.mout, "VERMOUTH")) {
		menudlg_setval(DID_OUT_VERMOUTH, 1);
	}
	else {
		menudlg_setval(DID_OUT_MIDIDEV, 1);
	}

	/* Module */
	menudlg_setval(DID_MDL, module2index(np2oscfg.mpu.mdl));

	setportstr();
	setmdlstr();
}

static void dlgupdate(void) {

	UINT	update;
	UINT8	mpuopt;
	UINT	val;

	update = 0;

	/* Enable */
	val = menudlg_getval(DID_MPU_EN);
	if (np2cfg.mpuenable != (UINT8)val) {
		np2cfg.mpuenable = (UINT8)val;
		update |= SYS_UPDATECFG;
	}

	/* Build mpuopt from port slider + interrupt radio */
	mpuopt = (UINT8)((menudlg_getval(DID_PORT) & 0x0f) << 4);
	if (menudlg_getval(DID_INT0)) {
		mpuopt |= 0;
	}
	else if (menudlg_getval(DID_INT1)) {
		mpuopt |= 1;
	}
	else if (menudlg_getval(DID_INT5)) {
		mpuopt |= 3;
	}
	else {
		mpuopt |= 2;	/* INT2 default */
	}
	if (np2cfg.mpuopt != mpuopt) {
		np2cfg.mpuopt = mpuopt;
		update |= SYS_UPDATECFG | SYS_UPDATEMIDI;
	}

	/* MIDI output */
	{
		const char *mout;
		if (menudlg_getval(DID_OUT_VERMOUTH)) {
			mout = "VERMOUTH";
		}
		else {
			mout = "MIDI-OUT device";
		}
		if (milstr_cmp(np2oscfg.mpu.mout, mout)) {
			milstr_ncpy(np2oscfg.mpu.mout, mout,
						NELEMENTS(np2oscfg.mpu.mout));
			update |= SYS_UPDATEOSCFG | SYS_UPDATEMIDI;
		}
	}

	/* Module */
	val = menudlg_getval(DID_MDL);
	if (val >= MIDI_MDLCOUNT) {
		val = MIDI_MDLCOUNT - 1;
	}
	if (milstr_cmp(np2oscfg.mpu.mdl, str_mdlnames[val])) {
		milstr_ncpy(np2oscfg.mpu.mdl, str_mdlnames[val],
					NELEMENTS(np2oscfg.mpu.mdl));
		update |= SYS_UPDATEOSCFG | SYS_UPDATEMIDI;
	}

	sysmng_update(update);
}

int dlgmidi_cmd(int msg, MENUID id, long param) {

	switch(msg) {
		case DLGMSG_CREATE:
			dlginit();
			break;

		case DLGMSG_COMMAND:
			switch(id) {
				case DID_OK:
					dlgupdate();
					menubase_close();
					break;

				case DID_CANCEL:
					menubase_close();
					break;

				case DID_PORT:
					setportstr();
					break;

				case DID_MDL:
					setmdlstr();
					break;
			}
			break;

		case DLGMSG_CLOSE:
			menubase_close();
			break;
	}
	(void)param;
	return(0);
}
