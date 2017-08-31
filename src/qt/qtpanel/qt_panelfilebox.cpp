#include <QWidget>
#include <QBrush>
#include <QPen>
#include <QRect>
#include <QPixmap>
#include <QPainter>
#include <QTimer>

#include <QDebug>

#include "define.h"
#include "strutil.h"
#include "file.h"
#include "colorcfgload.h"
#include "mlscfgload.h"

#include "qt_panel.h"

#include "qt_dialog.h"
#include "qt_panelfilebox.h"

using namespace MLSUTIL;
using namespace MLS;

void Position::Resize() {
    if (_ObjPosY == TOP)
        y = 0;
    else if (_ObjPosY == MIDDLE)
        y = (_pWidget->height() / 2) - (height / 2);
    else if (_ObjPosY == BOTTOM)
        y = _pWidget->height() - height;

    if (_ObjPosX == LEFT)
        x = 0;
    else if (_ObjPosX == MIDDLE)
        x = (_pWidget->width() / 2) - (width / 2);
    else if (_ObjPosX == RIGHT)
        x = _pWidget->width() - width;

    if (_pWidget->width() < x) x = _pWidget->width();
    if (_pWidget->height() < y) y = _pWidget->height();

    if (_pWidget->width() < x + width) width = _pWidget->width() - x;
    if (_pWidget->height() < y + height) height = _pWidget->height() - y;
}

Qt_FileBox::Qt_FileBox() :
        _focusColor(QColor(Qt::blue)),
        _fontColor(QColor(Qt::black)),
        _backColor(QColor(Qt::white)),
        _lineColor(QColor(Qt::white)) {
    _bFocus = false;
    _nDrawType = 0;
    _nNumber = -1;
    _bShowFileOwner = false;
    _bTitle = false;
    _nDefFontSize = 12;
    _bTitleButton = false;
}

Qt_FileBox::~Qt_FileBox() {
    for (int n = 0; n < _vTitleButtonList.size(); n++) {
        if (_vTitleButtonList[n]->_pButton)
            delete _vTitleButtonList[n]->_pButton;
        delete _vTitleButtonList[n];

        qDebug() << "Qt_FileBox _vTitleButtonList delete.";
    }
}

void Qt_FileBox::setFileBox(const QColor &focusColor,
                            const QColor &fontColor,
                            const QColor &backColor,
                            const QColor &lineColor,
                            bool bTitle,
                            bool bTitleButton) {
    _backColor = backColor;
    _fontColor = fontColor;
    _focusColor = focusColor;
    _lineColor = lineColor;
    _bTitle = bTitle;
    _bTitleButton = bTitleButton;

    if (_vTitleButtonList.size()) {
        qDebug() << "_vTitleButtonList size !!!!!!!!!!!" << _vTitleButtonList.size();
    }

    for (int n = 0; n < _vTitleButtonList.size(); n++) {
        if (_vTitleButtonList[n]->_pButton)
            delete _vTitleButtonList[n]->_pButton;
        delete _vTitleButtonList[n];
    }

    _vTitleButtonList.clear();
    _pFile = 0;
}

void Qt_FileBox::SetFile(MLS::File *pFile, bool bFocus, int nNumber, bool bFileOwner, int nDefFontWidth) {
    _bFocus = bFocus;
    _nNumber = nNumber;
    _bShowFileOwner = bFileOwner;

    _nDefFontSize = nDefFontWidth;

    if (pFile) {
        _pFile = pFile;
        if (_pFile->bDir)
            _pFile->tColor = g_tColorCfg.GetColorEntry("Dir");
    }
}

void Qt_FileBox::SizeView(QRect &box) {
    QFontMetrics fm(_pWidget->font());

    QPainter painter(_pWidget);

    if (!_bTitleButton)
        box.setWidth((_nDefFontSize * 9) + 4);

    if (!_bTitle) {
        if (_pFile->uSize >= 1000000000) {
            QString sSize, sGiga("G");
            sSize.sprintf("%.2f", (float) _pFile->uSize / 1073741824);

            QRect gigaRect = box;
            painter.setPen(QPen(Qt::red)); // Color
            gigaRect.setX(box.x() + box.width() - fm.width(sGiga) - 2);
            gigaRect.setWidth(fm.width(sGiga));
            painter.drawText(gigaRect, Qt::AlignVCenter | Qt::AlignLeft, sGiga);

            QRect sizetxt = box;
            sizetxt.setX(gigaRect.x() - fm.width(sSize));
            sizetxt.setWidth(fm.width(sSize));
            painter.setPen(QPen(_fontColor)); // Color
            painter.drawText(sizetxt, Qt::AlignVCenter | Qt::AlignLeft, sSize);
        } else if (_pFile->uSize >= 10000000) {
            QString sSize, sMega("M");
            sSize.sprintf("%.2f", (float) _pFile->uSize / 1048576);

            QRect megaRect = box;
            painter.setPen(QPen(Qt::red)); // Color
            megaRect.setX(box.x() + box.width() - fm.width(sMega) - 2);
            megaRect.setWidth(fm.width(sMega));
            painter.drawText(megaRect, Qt::AlignVCenter | Qt::AlignLeft, sMega);

            QRect sizetxt = box;
            sizetxt.setX(megaRect.x() - fm.width(sSize));
            sizetxt.setWidth(fm.width(sSize));
            painter.setPen(QPen(_fontColor)); // Color
            painter.drawText(sizetxt, Qt::AlignVCenter | Qt::AlignLeft, sSize);
        } else {
            QString sSize = MLSUTIL::toregular(_pFile->uSize).c_str();
            QRect sizetxt = box;
            sizetxt.setX(box.x() + box.width() - fm.width(sSize));

            //qDebug("Size [%s] [%d]", (const char*)sSize, fm.width( sSize ));
            sizetxt.setWidth(fm.width(sSize));
            painter.setPen(QPen(_fontColor)); // Color
            painter.drawText(sizetxt, Qt::AlignVCenter | Qt::AlignLeft, sSize);
        }
    } else {
        QString str("Size");

        if (!_bTitleButton) {
            QRect sizetxt = box;
            sizetxt.setX(box.x() + box.width() - fm.width(str) - 2);
            painter.setPen(QPen(_fontColor));
            painter.drawText(sizetxt, Qt::AlignVCenter | Qt::AlignLeft, QObject::tr(str));
        } else {
            /*
            QPushButton* pButton = new QPushButton( _pWidget );
            pButton->setGeometry( box );
            pButton->setText( QObject::tr( str ) );
            pButton->connect( pButton, SIGNAL( clicked() ), this, SLOT( SizeClicked() ) );

            _vTitleButtonList.push_back( pButton );
            */
        }
    }
}

void Qt_FileBox::FileView(const QRect &box) {
    QPainter painter(_pWidget);

    if (!_bTitle) {
        QFontMetrics fm(_pWidget->font());
        QRect filebox(box), extbox(box);

        QString sExt(_pFile->Ext().c_str());
        QString sName;

        if (_bExtBind)
            sName = _pFile->sName.substr(0, _pFile->sName.size() - sExt.length() - 1).c_str();
        else
            sName = _pFile->sName.c_str();

        int nFontWidth = fm.width(QObject::tr(sName));
        if (nFontWidth > filebox.width()) {
            int nMaxLength = sName.length();

            do {
                sName = sName.left(nMaxLength);
                sName.append("...");
                nMaxLength--;
            } while (fm.width(QObject::tr(sName)) >= filebox.width() - 2);
        }

        //painter.fillRect( textBox, QBrush( Qt::blue ) );
        painter.setPen(QPen(LinMGlobal::GetColor(_pFile->tColor.font)));
        painter.drawText(filebox, Qt::AlignVCenter | Qt::AlignLeft, QObject::tr(sName));
    } else {
        QString str("Filename");

        if (!_bTitleButton) {
            painter.setPen(QPen(_fontColor));
            painter.drawText(box, Qt::AlignVCenter | Qt::AlignLeft, QObject::tr(str));
        } else {
            qDebug() << "FileView Title";
            _vTitleButtonList.push_back(new TitleButtonItem(_pWidget, str, box));
            QTimer::singleShot(100, this, SLOT(slotTitleShow()));
        }
    }
}

void Qt_FileBox::ExtView(QRect &box) {
    QPainter painter(_pWidget);

    if (!_bTitle) {
        if (!_bExtBind) return;

        QFontMetrics fm(_pWidget->font());
        QString sExt(_pFile->Ext().c_str());

        int nFontWidth = fm.width(QObject::tr(sExt));
        if (nFontWidth > box.width()) {
            int nMaxLength = sExt.length();

            do {
                sExt = sExt.left(nMaxLength);
                sExt.append("...");
                nMaxLength--;
            } while (fm.width(QObject::tr(sExt)) >= box.width() - 2);
        }

        painter.setPen(QPen(_lineColor));
        painter.drawLine(box.x(), box.y(),
                         box.x(), box.y() + box.height());

        box.setX(box.x() + 2);
        painter.setPen(QPen(LinMGlobal::GetColor(_pFile->tColor.font)));
        painter.drawText(box, Qt::AlignVCenter | Qt::AlignLeft, QObject::tr(sExt));
    }
    /*
    else
    {
        QString		str("Ext");
        painter.setPen( QPen( _fontColor ) );
        painter.drawText( box, Qt::AlignVCenter | Qt::AlignLeft, QObject::tr(str) );
    }
    */
}

void Qt_FileBox::IconView(QRect &iconBox) {
    QPainter painter(_pWidget);

    if (!_bTitle) {
        const QPixmap &tIconData = LinMGlobal::GetSmallIcon(_pFile->sTmp3.c_str());

        if (!tIconData.isNull()) {
            iconBox.setWidth(tIconData.width() + 5);
            painter.drawPixmap(iconBox.x(),
                               iconBox.y() + ((iconBox.height() - tIconData.height()) / 2),
                               tIconData);
        } else
            iconBox.setWidth(25);
    }
}

void Qt_FileBox::AttrView(QRect &box) {
    QPainter painter(_pWidget);

    if (!_bTitle) {
        painter.setPen(QPen(_fontColor)); // Color
        painter.drawText(box, Qt::AlignVCenter | Qt::AlignLeft, QObject::tr(_pFile->sAttr.c_str()));
    } else {
        QString str("Permission");

        if (!_bTitleButton) {
            painter.setPen(QPen(_fontColor));
            painter.drawText(box, Qt::AlignVCenter | Qt::AlignLeft, QObject::tr(str));
        } else {
            qDebug() << "AttrView Title";

            /*
            QPushButton* pButton = new QPushButton( _pWidget );
            pButton->setGeometry( box );
            pButton->setText( QObject::tr( str ) );
            pButton->connect( pButton, SIGNAL( clicked() ), this, SLOT( AttrClicked() ) );
            //pButton->show();

            _vTitleButtonList.push_back( pButton );
            */
        }
    }
}

void Qt_FileBox::DateView(QRect &box) {
    QPainter painter(_pWidget);

    if (!_bTitle) {
        painter.setPen(QPen(_fontColor)); // Color
        painter.drawText(box, Qt::AlignVCenter | Qt::AlignLeft, QObject::tr(_pFile->sDate.c_str()));
    } else {
        QString str("Date");

        if (!_bTitleButton) {
            painter.setPen(QPen(_fontColor));
            painter.drawText(box, Qt::AlignVCenter | Qt::AlignLeft, QObject::tr(str));
        } else {
            qDebug() << "DateView Title";
            /*
            QPushButton* pButton = new QPushButton( _pWidget );
            pButton->setGeometry( box );
            pButton->setText( QObject::tr( str ) );
            pButton->connect( pButton, SIGNAL( clicked() ), this, SLOT( DateClicked() ) );
        //	pButton->show();

            _vTitleButtonList.push_back( pButton );
            */
        }
    }
}

void Qt_FileBox::TimeView(QRect &box) {
    QPainter painter(_pWidget);

    if (!_bTitle) {
        painter.setPen(QPen(_fontColor)); // Color
        painter.drawText(box, Qt::AlignVCenter | Qt::AlignLeft, QObject::tr(_pFile->sTime.c_str()));
    } else {
        QString str("Time");

        if (!_bTitleButton) {
            painter.setPen(QPen(_fontColor));
            painter.drawText(box, Qt::AlignVCenter | Qt::AlignLeft, QObject::tr(str));
        } else {
            /*
            qDebug() << "TimeView Title";
            QPushButton* pButton = new QPushButton( _pWidget );
            pButton->setGeometry( box );
            pButton->setText( QObject::tr( str ) );
            pButton->connect( pButton, SIGNAL( clicked() ), this, SLOT( TimeClicked() ) );
            //pButton->show();

            _vTitleButtonList.push_back( pButton );
            */
        }
    }
}

void Qt_FileBox::OwnerView(QRect &box) {
    QPainter painter(_pWidget);

    if (!_bTitle) {
        int nMaxLength = box.width() / _nDefFontSize;

        QString sOwner = _pFile->sOwner.c_str();
        if (sOwner.length() >= nMaxLength) {
            sOwner = sOwner.left(nMaxLength - 4);
            sOwner.append("...");
        }

        painter.setPen(QPen(_fontColor)); // Color
        painter.drawText(box, Qt::AlignVCenter | Qt::AlignLeft, QObject::tr(sOwner));
    } else {
        QString str("Owner");

        if (!_bTitleButton) {
            painter.setPen(QPen(_fontColor));
            painter.drawText(box, Qt::AlignVCenter | Qt::AlignLeft, QObject::tr(str));
        } else {
            qDebug() << "OwnerView Title";
            /*
            QPushButton* pButton = new QPushButton( _pWidget );
            pButton->setGeometry( box );
            pButton->setText( QObject::tr( str ) );
            pButton->connect( pButton, SIGNAL( clicked() ), this, SLOT( OwnerClicked() ) );
            //pButton->show();

            _vTitleButtonList.push_back( pButton );
            */
        }
    }
}

void Qt_FileBox::GroupView(QRect &groupBox) {
    QPainter painter(_pWidget);

    if (!_bTitle) {
        int nMaxLength = groupBox.width() / _nDefFontSize;

        QString sGroup = _pFile->sGroup.c_str();
        if (sGroup.length() >= nMaxLength) {
            sGroup = sGroup.left(nMaxLength - 4);
            sGroup.append("...");
        }

        painter.setPen(QPen(_fontColor)); // Color
        painter.drawText(groupBox, Qt::AlignVCenter | Qt::AlignLeft, QObject::tr(sGroup));
    } else {
        QString str("Group");

        if (!_bTitleButton) {
            painter.setPen(QPen(_fontColor));
            painter.drawText(groupBox, Qt::AlignVCenter | Qt::AlignLeft, QObject::tr(str));
        } else {
            /*
            qDebug() << "GroupView Title";
            QPushButton* pButton = new QPushButton( _pWidget );

            pButton->setGeometry( groupBox );
            pButton->setText( QObject::tr( str ) );
            pButton->connect( pButton, SIGNAL( clicked() ), this, SLOT( GroupClicked() ) );
            //pButton->show();

            _vTitleButtonList.push_back( pButton );
            */
        }
    }
}

void Qt_FileBox::DrawTabBox(const string &str, QRect &viewRect) {
    if (str == "Icon") IconView(viewRect);
    if (str == "File") FileView(viewRect);
    if (str == "Ext") ExtView(viewRect);
    if (str == "Size") SizeView(viewRect);
    if (str == "Attr") AttrView(viewRect);
    if (str == "Date") DateView(viewRect);
    if (str == "Time") TimeView(viewRect);
    if (str == "Owner") OwnerView(viewRect);
    if (str == "Group") GroupView(viewRect);
}

void Qt_FileBox::SortViewTab(const vector<TabViewInfo> &vViewList, const QRect &viewRect) {
    int nWidth = 5; // first X Position.
    int nCur = -1, n = 0, nFileX = 0;
    string str;
    QRect infoBox = viewRect;
    int nFileWidth = 0;

    QPainter painter(_pWidget);

    for (n = 0; n < vViewList.size(); n++) {
        TabViewInfo const &tViewInfo = vViewList[n];

        if (tViewInfo.sName == "File") {
            nCur = n;

            if (!_bTitle || !_bTitleButton)
                nFileX = infoBox.x() + nWidth;
            else
                nFileX = infoBox.x();
            break;
        }
        infoBox.setX(infoBox.x() + nWidth);
        if (nWidth != 0)
            infoBox.setWidth(tViewInfo.nWidth);

        QRect txtBox = infoBox;

        if (!_bTitle || !_bTitleButton) {
            txtBox.setX(txtBox.x() + 3);
            txtBox.setWidth(txtBox.width() - 3);
        }

        DrawTabBox(tViewInfo.sName, txtBox);

        nWidth = infoBox.width();
        nFileWidth = nFileWidth + nWidth;

        if (!_bTitle || !_bTitleButton) {
            if (tViewInfo.sName != "Icon" && n != vViewList.size() - 1) {
                if (_bFocus && _pWidget->focusWidget() == _pWidget)
                    painter.setPen(QPen(qRgba(_lineColor.red(),
                                              _lineColor.green(),
                                              _lineColor.blue(),
                                              200)));
                else
                    painter.setPen(QPen(_lineColor));

                painter.drawLine(infoBox.x() + infoBox.width(), infoBox.y(),
                                 infoBox.x() + infoBox.width(), infoBox.y() + infoBox.height());
            }
        }
    }

    if (nCur != -1 && vViewList.size() > nCur + 1) {
        infoBox = viewRect;
        infoBox.setX(viewRect.x() + viewRect.width());
        for (n = vViewList.size() - 1; n >= nCur + 1; n--) {
            TabViewInfo const &tViewInfo = vViewList[n];
            infoBox.setX(infoBox.x() - tViewInfo.nWidth);
            if (nWidth != 0)
                infoBox.setWidth(tViewInfo.nWidth);

            QRect txtBox = infoBox;

            if (!_bTitle || !_bTitleButton) {
                txtBox.setX(txtBox.x() + 3);
                txtBox.setWidth(txtBox.width() - 3);
            } else {
                if (n == vViewList.size() - 1) {
                    txtBox.setWidth(txtBox.width() - 3);
                }
            }

            DrawTabBox(tViewInfo.sName, txtBox);

            nWidth = infoBox.width();
            nFileWidth = nFileWidth + tViewInfo.nWidth;

            if (tViewInfo.sName != "Icon" && n != vViewList.size() - 1) {
                if (!_bTitle || !_bTitleButton) {

                    if (_bFocus && _pWidget->focusWidget() == _pWidget)
                        painter.setPen(QPen(qRgba(_lineColor.red(),
                                                  _lineColor.green(),
                                                  _lineColor.blue(),
                                                  200)));
                    else
                        painter.setPen(QPen(_lineColor));

                    painter.drawLine(infoBox.x() + infoBox.width(), infoBox.y(),
                                     infoBox.x() + infoBox.width(), infoBox.y() + infoBox.height());
                }
            }
        }
    }

    if (nCur != -1) {
        infoBox.setX(nFileX);
        infoBox.setWidth(viewRect.width() - nFileWidth);
        QRect FileBox = infoBox;
        if (!_bTitle || !_bTitleButton) {
            FileBox.setX(FileBox.x() + 3);
            FileBox.setWidth(FileBox.width() - 3);
        }
        DrawTabBox("File", FileBox);

        /*
        if ( !_bFocus || _pWidget->focusWidget() != _pWidget )
            painter.setPen( QPen( qRgba( _lineColor.red(),
                                         _lineColor.green(),
                                         _lineColor.blue(),
                                         200 ) ) );
        else
            painter.setPen( QPen( _lineColor ) );

        painter.drawLine( 	infoBox.x() + infoBox.width(), infoBox.y(),
                            infoBox.x() + infoBox.width(), infoBox.y() + infoBox.height());
        */
    }
}

void Qt_FileBox::Draw() {
    QPainter painter(_pWidget);

    //painter.setBackgroundMode( Qt::TransparentMode );

    _viewRect = QRect(x, y, width, height);

    if (!_bTitle) {
        if (!_bFocus || _pWidget->focusWidget() != _pWidget) {
            painter.setPen(QPen(_backColor));
            painter.setBrush(QBrush(_backColor, Qt::SolidPattern));
            painter.fillRect(_viewRect, QBrush(_backColor));
        } else {
            painter.setPen(QPen(_fontColor));
            painter.setBrush(QBrush(_focusColor, Qt::SolidPattern));
            QRect viewRect = QRect(x, y, width - 1, height - 1);
            painter.drawRoundRect(viewRect, 2);
        }

        /*
        if ( _bMouseFocus )
            painter.fillRect( _viewRect, QBrush( qAlpha( _backColor.rgb() ) ) );
        */
        if (!_pFile) return;

        _bExtBind = g_tCfg.GetExtBind(_pFile->Ext()).empty() ? false : true;
    } else {
        if (!_bTitleButton) {
            painter.setPen(QPen(_backColor));
            painter.setBrush(QBrush(_backColor, Qt::SolidPattern));

            QRect viewRect = QRect(x, y, width - 1, height - 1);
            painter.drawRoundRect(viewRect, 3);
        }
    }

    //QPen pen2(Qt::red, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    vector<TabViewInfo> vTabView;

    if (width <= 300) {
        if (!_bTitle || !_bTitleButton) {
            vTabView.push_back(TabViewInfo("Icon", 20));
            vTabView.push_back(TabViewInfo("File"));
            vTabView.push_back(TabViewInfo("Ext", (_nDefFontSize * 4)));
        } else
            vTabView.push_back(TabViewInfo("File"));
    } else if (width <= 400) {
        if (!_bTitle || !_bTitleButton) {
            vTabView.push_back(TabViewInfo("Icon", 20));
            vTabView.push_back(TabViewInfo("File"));
            vTabView.push_back(TabViewInfo("Ext", (_nDefFontSize * 4)));
            vTabView.push_back(TabViewInfo("Size", (_nDefFontSize * 11)));
        } else {
            vTabView.push_back(TabViewInfo("File"));
            vTabView.push_back(TabViewInfo("Size", (_nDefFontSize * 11)));
        }
    } else if (width <= 500) {
        if (!_bTitle || !_bTitleButton) {
            vTabView.push_back(TabViewInfo("Icon", 20));
            vTabView.push_back(TabViewInfo("File"));
            vTabView.push_back(TabViewInfo("Ext", (_nDefFontSize * 4)));
            vTabView.push_back(TabViewInfo("Size", (_nDefFontSize * 11)));
        } else {
            vTabView.push_back(TabViewInfo("File"));
            vTabView.push_back(TabViewInfo("Size", (_nDefFontSize * 11)));
        }
        vTabView.push_back(TabViewInfo("Date", (_nDefFontSize * 9)));
        vTabView.push_back(TabViewInfo("Time", (_nDefFontSize * 6)));
    } else if (width <= 650) {
        if (!_bTitle || !_bTitleButton) {
            vTabView.push_back(TabViewInfo("Icon", 20));
            vTabView.push_back(TabViewInfo("File"));
            vTabView.push_back(TabViewInfo("Ext", (_nDefFontSize * 4)));
            vTabView.push_back(TabViewInfo("Size", (_nDefFontSize * 11)));
        } else {
            vTabView.push_back(TabViewInfo("File"));
            vTabView.push_back(TabViewInfo("Size", (_nDefFontSize * 11)));
        }

        vTabView.push_back(TabViewInfo("Date", (_nDefFontSize * 9)));
        vTabView.push_back(TabViewInfo("Time", (_nDefFontSize * 6)));
        vTabView.push_back(TabViewInfo("Attr", (_nDefFontSize * 11)));
    } else {
        if (!_bTitle || !_bTitleButton) {
            vTabView.push_back(TabViewInfo("Icon", 20));
            vTabView.push_back(TabViewInfo("File"));
            vTabView.push_back(TabViewInfo("Ext", (_nDefFontSize * 4)));
            vTabView.push_back(TabViewInfo("Size", (_nDefFontSize * 11)));
        } else {
            vTabView.push_back(TabViewInfo("File"));
            vTabView.push_back(TabViewInfo("Size", (_nDefFontSize * 11)));
        }
        vTabView.push_back(TabViewInfo("Date", (_nDefFontSize * 9)));
        vTabView.push_back(TabViewInfo("Time", (_nDefFontSize * 6)));
        vTabView.push_back(TabViewInfo("Attr", (_nDefFontSize * 11)));
        vTabView.push_back(TabViewInfo("Owner", (_nDefFontSize * 10)));
        vTabView.push_back(TabViewInfo("Group", (_nDefFontSize * 10)));
    }

    SortViewTab(vTabView, _viewRect);
}

void Qt_FileBox::FilenameClicked() {

}

void Qt_FileBox::SizeClicked() {
}

void Qt_FileBox::IconClicked() {
}

void Qt_FileBox::AttrClicked() {
}

void Qt_FileBox::DateClicked() {
}

void Qt_FileBox::TimeClicked() {
}

void Qt_FileBox::OwnerClicked() {
}

void Qt_FileBox::GroupClicked() {
}

void Qt_FileBox::slotTitleShow() {
    qDebug() << "slotTitleShow :: " << _vTitleButtonList.size();

    for (int n = 0; n < _vTitleButtonList.size(); n++) {
        TitleButtonItem *pItem = _vTitleButtonList[n];

        QPushButton *pButton = new QPushButton(pItem->_pParent);

        pButton->setGeometry(pItem->_rect);
        pButton->setText(QObject::tr(pItem->_strTitleName));

        QString strFuncName;
        strFuncName.sprintf("%d%sClicked()", QSLOT_CODE, (const char *) pItem->_strTitleName.toAscii());

        pButton->connect(pButton, SIGNAL(clicked()), this, (const char *) strFuncName.toAscii());
        pButton->show();

        pItem->_pButton = pButton;
    }
}

void Qt_FileBox::checkAddedTitle(TitleButtonItem *pItemTarget) {
    /*
    for ( int n = 0; n < _vTitleButtonList.size(); n++ )
    {
        if ( _vTitleButtonList[n]->_rect != pItemTarget->_rect &&
             _vTitleButtonList[n]->_strTitleName != pItemTarget->_strTitleName )
        {

        }
    }

    QPushButton* pButton = new QPushButton( pItem->_pParent );
    pButton->setGeometry( itemTarget._rect );
    pButton->setText( QObject::tr( itemTarget._strTitleName ) );
    */
}
