#pragma once

#include <QComboBox>
#include <QIcon>
#include <QMenu>
#include <QStyle>
#include <QString>
#include <QVariant>
#include <QWidget>

#include "../xcore/xcore.h"

// Enumerate a resource kind, precompute the user/system icon pair once, and
// invoke `add(icon, entry)` for every matching entry. This is the common spine
// shared by the combo, menu, and xTreeBox fillers — each specializes only in
// what they do per item. Icons are looked up via `styleSrc->style()` so the
// host widget decides the icon theme.
template <typename Pred, typename Adder>
inline void forEachResource(QWidget *styleSrc, ResourceKind kind,
                            Pred &&pred, Adder &&add) {
	const QIcon userIcon = styleSrc->style()->standardIcon(QStyle::SP_DirHomeIcon);
	const QIcon sysIcon  = styleSrc->style()->standardIcon(QStyle::SP_ComputerIcon);
	for (const auto &e : conf.path.enumerateRecursive(kind, std::forward<Pred>(pred))) {
		add(e.origin == ResourceOrigin::User ? userIcon : sysIcon, e);
	}
}

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
	forEachResource(box, kind, std::forward<Pred>(pred),
		[&](const QIcon &icon, const ResolvedEntry &e) {
			const QString name = toQString(e.name);
			box->addItem(icon, name,
			             commonData.isValid() ? commonData : QVariant(name));
		});
}

// Populate a QMenu with checkable actions for a resource kind. Each action's
// data is the filename; the action whose filename equals `current` is marked
// checked.
template <typename Pred>
inline void fillCheckableMenuFromResources(QMenu *menu, ResourceKind kind,
                                           Pred &&pred,
                                           const QString &current) {
	forEachResource(menu, kind, std::forward<Pred>(pred),
		[&](const QIcon &icon, const ResolvedEntry &e) {
			const QString name = toQString(e.name);
			QAction *act = menu->addAction(icon, name);
			act->setData(name);
			act->setCheckable(true);
			act->setChecked(name == current);
		});
}
