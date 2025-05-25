#include "emulwin.h"
#include "filer.h"

// socket

#ifdef USENETWORK

void MainWin::openServer() {
	int i = 0;
	while (!srv.listen(QHostAddress::LocalHost, (conf.port + i) & 0xffff) && (i < 0x10000)) {
		i++;
	}
	if (!srv.isListening()) {
		shitHappens("Listen server can't start");
	} else {
		printf("Listening port %i\n", (conf.port + i) & 0xffff);
	}
}

void MainWin::closeServer() {
	if (srv.isListening()) {
		foreach(QTcpSocket* sock, clients) {
			sock->close();
			sock->deleteLater();
		}
		srv.close();
	}
}

void MainWin::connected() {
	QTcpSocket* sock = srv.nextPendingConnection();
	clients.append(sock);
	sock->write("Who's there?\n> ");
	connect(sock,SIGNAL(destroyed()),this,SLOT(disconnected()));
	connect(sock,SIGNAL(readyRead()),this,SLOT(socketRead()));
}

void MainWin::disconnected() {
	QTcpSocket* sock = (QTcpSocket*)sender();
	disconnect(sock);
	clients.removeAll(sock);
	sock->deleteLater();
}

static char dasmbuf[256];
extern int dasmrd(int adr, void* ptr);
extern int str_to_adr(Computer* comp, QString str);

void MainWin::socketRead() {
	QTcpSocket* sock = (QTcpSocket*)sender();
	QByteArray arr = sock->readAll();
	QString com(arr);
	com = com.remove("\n");
	com = com.remove("\r");
	QStringList prm = com.split(" ",X_SkipEmptyParts);
	if (prm.size() == 0) return;
	com = prm[0];
	xMnem mnm;
	bool f;
	int adr, cnt, val;
	Computer* comp = conf.prof.cur->zx;
	// and do something with this
	if ((com == "debug") || (com == "dbg")) {
		doDebug();
	} else if (com == "closedbg") {
		emit s_debug_off();
	} else if (com == "quit") {
		close();
	} else if (com == "exit") {
		sock->close();
	} else if (com == "pause") {
		pause(true, PR_PAUSE);
	} else if ((com == "cont") || (com == "unpause")) {
		pause(false, PR_PAUSE);
	} else if (com == "step") {
		emit s_step();
	} else if (com == "reset") {
		compReset(comp, RES_DEFAULT);
	} else if (com == "cpu") {
		//sock->write(getCoreName(comp->cpu->type));
		sock->write(comp->cpu->core->name);
		sock->write("\n");
	} else if (com == "getreg") {
		if (prm.size() > 1) {
			val = cpu_get_reg(comp->cpu, prm[1].toUpper().toLocal8Bit().data(), &f);
			if (f) {
				sock->write(QString::number(val, 16).toUpper().toUtf8());
				sock->write("\r\n");
			} else {
				sock->write("Wrong register name\r\n");
			}
		}
	} else if (com == "setreg") {
		if (prm.size() > 2) {
			if (!cpu_set_reg(comp->cpu, prm[1].toUpper().toLocal8Bit().data(), str_to_adr(comp, prm[2])))
				sock->write("Wrong register name\r\n");
		}
	} else if (com == "cpuregs") {
		xRegBunch rb = cpuGetRegs(comp->cpu);
		xRegister reg;
		for (cnt = 0; cnt < 32; cnt++) {
			reg = rb.regs[cnt];
			if ((reg.id != REG_NONE) && (reg.id != REG_EMPTY)) {
				sock->write(reg.name);
				sock->write(" : ");
				switch(reg.type & REG_TMASK) {
					case REG_BIT: sock->write(reg.value ? "1" : "0"); break;
					case REG_BYTE: sock->write(gethexbyte(reg.value).toUtf8()); break;
					case REG_WORD: sock->write(gethexword(reg.value).toUtf8()); break;
					case REG_24: sock->write(gethex6(reg.value).toUtf8()); break;
					case REG_32: sock->write(gethexint(reg.value).toUtf8()); break;
					default: sock->write("??"); break;
				}
				sock->write("\r\n");
			}
		}
	} else if (com == "load") {
		if (prm.size() > 1) {
			load_file(comp, prm[1].toLocal8Bit().data(), FG_ALL, 0);
		}
	} else if ((com == "poke") || (com == "memwr")) {
		if (prm.size() > 2) {
			adr = str_to_adr(comp, prm[1]);
			val = str_to_adr(comp, prm[2]);
			comp->hw->mwr(comp, adr, val & 0xff);
		}
	} else if ((com == "pokew") || (com == "memwrw")) {
		if (prm.size() > 2) {
			adr = str_to_adr(comp, prm[1]);
			val = str_to_adr(comp, prm[2]);
			comp->hw->mwr(comp, adr, val & 0xff);
			comp->hw->mwr(comp, adr+1, (val >> 8) & 0xff);
		}
	} else if (com == "memfill") {
		if (prm.size() > 3) {
			adr = str_to_adr(comp, prm[1]);
			cnt = str_to_adr(comp, prm[2]);
			val = str_to_adr(comp, prm[3]);
			while (cnt > 0) {
				comp->hw->mwr(comp, adr, val);
				adr++;
				cnt--;
			}
		}
	} else if (com == "memcopy") {
		if (prm.size() > 3) {
			adr = str_to_adr(comp, prm[1]);
			cnt = str_to_adr(comp, prm[2]);
			val = str_to_adr(comp, prm[3]);
			qDebug() << adr << cnt << val;
			while (cnt > 0) {
				if (adr > val) {
					comp->hw->mwr(comp, val, comp->hw->mrd(comp, adr, 0));		// dst < src
					adr++;
					val++;
				} else {								// dst >= src
					comp->hw->mwr(comp, val+cnt-1, comp->hw->mrd(comp, adr+cnt-1, 0));
				}
				cnt--;
			}
		}
	} else if (com == "disasm") {
		if (prm.size() > 1) {
			adr = str_to_adr(comp, prm[1]);
			cnt = (prm.size() > 2) ? prm[2].toInt() : 1;
			// qDebug() << cnt;
			while (cnt > 0) {
				sprintf(dasmbuf, "%.6X : ", adr);
				sock->write(dasmbuf);
				mnm = cpuDisasm(comp->cpu, comp->cpu->cs.base + adr, dasmbuf, dasmrd, comp);
				sock->write(dasmbuf);
				sock->write("\r\n");
				adr += mnm.len;
				cnt--;
			}
		}
	} else if (com == "dump") {
		if (prm.size() > 1) {
			adr = str_to_adr(comp, prm[1]);
			if (prm.size() > 2) {
				cnt = prm[2].toInt();
				if (cnt < 1) {
					cnt = 16;
				} else {
					cnt <<= 4;
				}
			} else {
				cnt = 16;
			}
			while (cnt > 0) {
				sprintf(dasmbuf, "%.6X : ", adr);
				sock->write(dasmbuf);
				dasmbuf[0] = 0x00;
				do {
					sprintf(dasmbuf, "%.2X ", dasmrd(comp->cpu->cs.base + adr, comp));
					sock->write(dasmbuf);
					adr++;
					cnt--;
				} while (cnt & 15);
				sock->write("\r\n");
			}
		}
	} else {
		sock->write("Unrecognized command\r\n");
	}
	if (com != "quit")
		sock->write("> ");
}

#else

void MainWin::connected() {}
void MainWin::disconnected() {}
void MainWin::socketRead() {}

#endif
