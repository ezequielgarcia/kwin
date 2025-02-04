/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2007 Rivo Laks <rivolaks@hot.ee>
Copyright (C) 2008 Lucas Murray <lmurray@undefinedfire.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "desktopgrid_config.h"
// KConfigSkeleton
#include "desktopgridconfig.h"
#include <config-kwin.h>
#include <kwineffects_interface.h>

#include <QAction>

#include <kconfiggroup.h>
#include <KActionCollection>
#include <KAboutData>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QVBoxLayout>

K_PLUGIN_FACTORY_WITH_JSON(DesktopGridEffectConfigFactory,
                           "desktopgrid_config.json",
                           registerPlugin<KWin::DesktopGridEffectConfig>();)

namespace KWin
{

DesktopGridEffectConfigForm::DesktopGridEffectConfigForm(QWidget* parent) : QWidget(parent)
{
    setupUi(this);
}

DesktopGridEffectConfig::DesktopGridEffectConfig(QWidget* parent, const QVariantList& args)
    :   KCModule(KAboutData::pluginData(QStringLiteral("desktopgrid")), parent, args)
{
    m_ui = new DesktopGridEffectConfigForm(this);

    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(m_ui);

    // Shortcut config. The shortcut belongs to the component "kwin"!
    m_actionCollection = new KActionCollection(this, QStringLiteral("kwin"));

    m_actionCollection->setComponentDisplayName(i18n("KWin"));
    m_actionCollection->setConfigGroup(QStringLiteral("DesktopGrid"));
    m_actionCollection->setConfigGlobal(true);

    QAction* a = m_actionCollection->addAction(QStringLiteral("ShowDesktopGrid"));
    a->setText(i18n("Show Desktop Grid"));
    a->setProperty("isConfigurationAction", true);
    KGlobalAccel::self()->setDefaultShortcut(a, QList<QKeySequence>() << Qt::CTRL + Qt::Key_F8);
    KGlobalAccel::self()->setShortcut(a, QList<QKeySequence>() << Qt::CTRL + Qt::Key_F8);

    m_ui->shortcutEditor->addCollection(m_actionCollection);


    m_ui->desktopNameAlignmentCombo->addItem(i18nc("Desktop name alignment:", "Disabled"), QVariant(Qt::Alignment()));
    m_ui->desktopNameAlignmentCombo->addItem(i18n("Top"), QVariant(Qt::AlignHCenter | Qt::AlignTop));
    m_ui->desktopNameAlignmentCombo->addItem(i18n("Top-Right"), QVariant(Qt::AlignRight | Qt::AlignTop));
    m_ui->desktopNameAlignmentCombo->addItem(i18n("Right"), QVariant(Qt::AlignRight | Qt::AlignVCenter));
    m_ui->desktopNameAlignmentCombo->addItem(i18n("Bottom-Right"), QVariant(Qt::AlignRight | Qt::AlignBottom));
    m_ui->desktopNameAlignmentCombo->addItem(i18n("Bottom"), QVariant(Qt::AlignHCenter | Qt::AlignBottom));
    m_ui->desktopNameAlignmentCombo->addItem(i18n("Bottom-Left"), QVariant(Qt::AlignLeft | Qt::AlignBottom));
    m_ui->desktopNameAlignmentCombo->addItem(i18n("Left"), QVariant(Qt::AlignLeft | Qt::AlignVCenter));
    m_ui->desktopNameAlignmentCombo->addItem(i18n("Top-Left"), QVariant(Qt::AlignLeft | Qt::AlignTop));
    m_ui->desktopNameAlignmentCombo->addItem(i18n("Center"), QVariant(Qt::AlignCenter));

    DesktopGridConfig::instance(KWIN_CONFIG);
    addConfig(DesktopGridConfig::self(), m_ui);
    connect(m_ui->kcfg_LayoutMode, qOverload<int>(&KComboBox::currentIndexChanged), this, &DesktopGridEffectConfig::layoutSelectionChanged);
    connect(m_ui->desktopNameAlignmentCombo, qOverload<int>(&KComboBox::currentIndexChanged), this, qOverload<>(&DesktopGridEffectConfig::changed));
    connect(m_ui->shortcutEditor, &KShortcutsEditor::keyChange, this, qOverload<>(&DesktopGridEffectConfig::changed));

    load();
    layoutSelectionChanged();
}

DesktopGridEffectConfig::~DesktopGridEffectConfig()
{
    // If save() is called undoChanges() has no effect
    m_ui->shortcutEditor->undoChanges();
}

void DesktopGridEffectConfig::save()
{
    m_ui->shortcutEditor->save();
    DesktopGridConfig::setDesktopNameAlignment(m_ui->desktopNameAlignmentCombo->itemData(m_ui->desktopNameAlignmentCombo->currentIndex()).toInt());
    KCModule::save();

    OrgKdeKwinEffectsInterface interface(QStringLiteral("org.kde.KWin"),
                                         QStringLiteral("/Effects"),
                                         QDBusConnection::sessionBus());
    interface.reconfigureEffect(QStringLiteral("desktopgrid"));
}

void DesktopGridEffectConfig::load()
{
    KCModule::load();
    m_ui->desktopNameAlignmentCombo->setCurrentIndex(m_ui->desktopNameAlignmentCombo->findData(QVariant(DesktopGridConfig::desktopNameAlignment())));
}

void DesktopGridEffectConfig::layoutSelectionChanged()
{
    if (m_ui->kcfg_LayoutMode->currentIndex() == DesktopGridEffect::LayoutCustom) {
        m_ui->layoutRowsLabel->setEnabled(true);
        m_ui->kcfg_CustomLayoutRows->setEnabled(true);
    } else {
        m_ui->layoutRowsLabel->setEnabled(false);
        m_ui->kcfg_CustomLayoutRows->setEnabled(false);
    }
}

void DesktopGridEffectConfig::defaults()
{
    KCModule::defaults();
    m_ui->desktopNameAlignmentCombo->setCurrentIndex(0);
}

} // namespace

#include "desktopgrid_config.moc"
