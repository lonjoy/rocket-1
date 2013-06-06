#include "rownumberview.h"

#include <QPaintEvent>
#include <QStylePainter>
#include <QStyleOptionButton>

void RowNumberView::paintEvent(QPaintEvent *event)
{
	QStylePainter painter(this);

	QRect rect;
	rect.setLeft(0);
	rect.setRight(width() - 1);

	int lineSpacing = painter.fontMetrics().lineSpacing();
	int startRow = event->rect().top() / lineSpacing;
	int stopRow = (event->rect().bottom() + lineSpacing - 1) / lineSpacing;
	for (int r = startRow; r < stopRow; ++r) {
		rect.setTop(r * lineSpacing);
		rect.setBottom((r + 1) * lineSpacing);

		if (r == rowHilight)
			painter.fillRect(rect, Qt::yellow);
		else
			painter.fillRect(rect, palette().button());

		QStyleOptionButton option;
		option.initFrom(this);
		option.state |= QStyle::State_Raised;
		option.rect = rect;
		painter.drawPrimitive(QStyle::PE_FrameButtonBevel, option);

		if (r % 8 == 0)
			painter.setPen(QColor(0, 0, 0));
		else if (r % 4 == 0)
			painter.setPen(QColor(64, 64, 64));
		else
			painter.setPen(QColor(128, 128, 128));

		QString temp = QString("%1").arg(r, 5, 16, QLatin1Char('0')).toUpper() + QString("h");
		painter.drawText(rect, temp);
	}
}
