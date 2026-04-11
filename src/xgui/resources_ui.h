#pragma once

#include <QComboBox>
#include <QIcon>
#include <QMenu>
#include <QStyle>
#include <QString>
#include <QVariant>

#include "../xcore/xcore.h"

// Populate a QComboBox with entries for a resource kind, using icons to mark
// each entry as writable (user) or read-only (system). The predicate filters
// the enumeration (typically byExtension({...})).
//
// If `commonData` is invalid (the default), each entry stores its filename as
// user data. If `commonData` is set, every entry stores that value as user
// data — useful when the caller uses user data as a type tag (e.g. shader
// list distinguishing "none" vs "shader").
template <typename Pred>
inline void fillComboFromResources(QComboBox *box, ResourceKind kind,
                                   Pred &&pred,
                                   const QVariant &commonData = QVariant()) {
	const QIcon userIcon = box->style()->standardIcon(QStyle::SP_DirHomeIcon);
	const QIcon sysIcon  = box->style()->standardIcon(QStyle::SP_ComputerIcon);
	for (const auto &e : conf.path.enumerateRecursive(kind, std::forward<Pred>(pred))) {
		const QString name = toQString(e.name);
		box->addItem(e.user ? userIcon : sysIcon, name,
		             commonData.isValid() ? commonData : QVariant(name));
	}
}

// Populate a QMenu with checkable actions for a resource kind. Each action's
// data is the filename; the action whose filename equals `current` is marked
// checked.
template <typename Pred>
inline void fillCheckableMenuFromResources(QMenu *menu, ResourceKind kind,
                                           Pred &&pred,
                                           const QString &current) {
	const QIcon userIcon = menu->style()->standardIcon(QStyle::SP_DirHomeIcon);
	const QIcon sysIcon  = menu->style()->standardIcon(QStyle::SP_ComputerIcon);
	for (const auto &e : conf.path.enumerateRecursive(kind, std::forward<Pred>(pred))) {
		const QString name = toQString(e.name);
		QAction *act = menu->addAction(e.user ? userIcon : sysIcon, name);
		act->setData(name);
		act->setCheckable(true);
		act->setChecked(name == current);
	}
}
