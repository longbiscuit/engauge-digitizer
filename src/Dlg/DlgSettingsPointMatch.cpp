#include "CmdMediator.h"
#include "CmdSettingsPointMatch.h"
#include "DlgSettingsPointMatch.h"
#include "Logger.h"
#include "MainWindow.h"
#include <QComboBox>
#include <QGraphicsEllipseItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGridLayout>
#include <QLabel>
#include <qmath.h>
#include <QPen>
#include <QSpinBox>
#include "ViewPreview.h"

const int POINT_SEPARATION_MAX = 1024;
const int POINT_SEPARATION_MIN = 2;
const int POINT_SIZE_MAX = 1024;
const int POINT_SIZE_MIN = 5;

DlgSettingsPointMatch::DlgSettingsPointMatch(MainWindow &mainWindow) :
  DlgSettingsAbstractBase ("Point Match", mainWindow),
  m_scenePreview (0),
  m_viewPreview (0),
  m_boxSize (0),
  m_modelPointMatchBefore (0),
  m_modelPointMatchAfter (0)
{
  QWidget *subPanel = createSubPanel ();
  finishPanel (subPanel);
}

void DlgSettingsPointMatch::createControls (QGridLayout *layout,
                                            int &row)
{
  QLabel *labelMinPointSeparation = new QLabel ("Minimum point separation (pixels):");
  layout->addWidget (labelMinPointSeparation, row, 1);

  m_spinMinPointSeparation = new QSpinBox;
  QString whatsThis = QString ("Select a minimum point separation in pixels.\n\n"
                               "Matched points must be separated from existing points by at least this number of pixels.\n\n"
                               "This value has a lower limit of %1 since overlapping points are too hard to separate")
                      .arg (POINT_SEPARATION_MIN);
  m_spinMinPointSeparation->setWhatsThis (whatsThis);
  m_spinMinPointSeparation->setMinimum (POINT_SEPARATION_MIN);
  m_spinMinPointSeparation->setMaximum (POINT_SEPARATION_MAX);
  connect (m_spinMinPointSeparation, SIGNAL (valueChanged (int)), this, SLOT (slotMinPointSeparation (int)));
  layout->addWidget (m_spinMinPointSeparation, row++, 2);

  QLabel *labelPointSize = new QLabel ("Maximum point size (pixels):");
  layout->addWidget (labelPointSize, row, 1);

  m_spinPointSize = new QSpinBox;
  m_spinPointSize->setWhatsThis (tr ("Select a maximum point size in pixels.\n\n"
                                     "Sample match points must fit within a square box, around the cursor, having width and height "
                                     "equal to this maximum.\n\n"
                                     "This size is also used to determine if a region of pixels that are on, in the processed image, "
                                     "should be ignored since that region is wider or taller than this limit.\n\n"
                                     "This value has a lower limit"));
  m_spinPointSize->setMinimum (POINT_SIZE_MIN);
  m_spinPointSize->setMaximum (POINT_SIZE_MAX);
  connect (m_spinPointSize, SIGNAL (valueChanged (int)), this, SLOT (slotMaxPointSize (int)));
  layout->addWidget (m_spinPointSize, row++, 2);

  QLabel *labelAcceptedPointColor = new QLabel ("Accepted point color:");
  layout->addWidget (labelAcceptedPointColor, row, 1);

  m_cmbAcceptedPointColor = new QComboBox;
  m_cmbAcceptedPointColor->setWhatsThis (tr ("Select a color for matched points that are accepted"));
  populateColorComboWithTransparent (*m_cmbAcceptedPointColor);
  connect (m_cmbAcceptedPointColor, SIGNAL (currentTextChanged (const QString &)), this, SLOT (slotAcceptedPointColor (const QString &)));
  layout->addWidget (m_cmbAcceptedPointColor, row++, 2);

  QLabel *labelRejectedPointColor = new QLabel ("Rejected point color:");
  layout->addWidget (labelRejectedPointColor, row, 1);

  m_cmbRejectedPointColor = new QComboBox;
  m_cmbRejectedPointColor->setWhatsThis (tr ("Select a color for matched points that are rejected"));
  populateColorComboWithTransparent (*m_cmbRejectedPointColor);
  connect (m_cmbRejectedPointColor, SIGNAL (currentTextChanged (const QString &)), this, SLOT (slotRejectedPointColor (const QString &)));
  layout->addWidget (m_cmbRejectedPointColor, row++, 2);

  QLabel *labelCandidatePointColor = new QLabel ("Candidate point color:");
  layout->addWidget (labelCandidatePointColor, row, 1);

  m_cmbCandidatePointColor = new QComboBox;
  m_cmbCandidatePointColor->setWhatsThis (tr ("Select a color for the point being decided upon"));
  populateColorComboWithTransparent (*m_cmbCandidatePointColor);
  connect (m_cmbCandidatePointColor, SIGNAL (currentTextChanged (const QString &)), this, SLOT (slotCandidatePointColor (const QString &)));
  layout->addWidget (m_cmbCandidatePointColor, row++, 2);
}

void DlgSettingsPointMatch::createPreview (QGridLayout *layout,
                                           int &row)
{
  QLabel *labelPreview = new QLabel ("Preview");
  layout->addWidget (labelPreview, row++, 0, 1, 4);

  m_scenePreview = new QGraphicsScene (this);
  m_viewPreview = new ViewPreview (m_scenePreview, this);
  m_viewPreview->setWhatsThis (tr ("Preview window shows how current settings affect "
                                   "point matching, and how the marked and candidate points are displayed.\n\nThe points are separated "
                                   "by the point separation value, and the maximum point size is shown as a box in the center"));
  m_viewPreview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_viewPreview->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_viewPreview->setMinimumHeight (MINIMUM_PREVIEW_HEIGHT);
  connect (m_viewPreview, SIGNAL (signalMouseMove (QPointF)), this, SLOT (slotMouseMove (QPointF)));

  layout->addWidget (m_viewPreview, row++, 0, 1, 4);
}

QWidget *DlgSettingsPointMatch::createSubPanel ()
{
  QWidget *subPanel = new QWidget ();
  QGridLayout *layout = new QGridLayout (subPanel);
  subPanel->setLayout (layout);

  layout->setColumnStretch(0, 1); // Empty column
  layout->setColumnStretch(1, 0); // Labels
  layout->setColumnStretch(2, 0); // Controls
  layout->setColumnStretch(3, 1); // Empty column

  int row = 0;
  createControls (layout, row);
  createPreview (layout, row);

  return subPanel;
}

void DlgSettingsPointMatch::handleOk ()
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsPointMatch::handleOk";

  CmdSettingsPointMatch *cmd = new CmdSettingsPointMatch (mainWindow (),
                                                          cmdMediator ().document(),
                                                          *m_modelPointMatchBefore,
                                                          *m_modelPointMatchAfter);
  cmdMediator ().push (cmd);

  hide ();
}

void DlgSettingsPointMatch::load (CmdMediator &cmdMediator)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsPointMatch::load";

  setCmdMediator (cmdMediator);

  // Cross check local and incoming values. If this asserts, either limits in this class are broken or default value is out of bounds
  Q_ASSERT (POINT_SEPARATION_MIN <= cmdMediator.document().modelPointMatch().minPointSeparation());
  Q_ASSERT (POINT_SEPARATION_MAX > cmdMediator.document().modelPointMatch().minPointSeparation());
  Q_ASSERT (POINT_SIZE_MIN <= cmdMediator.document().modelPointMatch().maxPointSize());
  Q_ASSERT (POINT_SIZE_MAX > cmdMediator.document().modelPointMatch().maxPointSize());

  // Flush old data
  if (m_modelPointMatchBefore != 0) {
    delete m_modelPointMatchBefore;
  }
  if (m_modelPointMatchAfter != 0) {
    delete m_modelPointMatchAfter;
  }

  // Save new data
  m_modelPointMatchBefore = new DocumentModelPointMatch (cmdMediator.document());
  m_modelPointMatchAfter = new DocumentModelPointMatch (cmdMediator.document());

  m_spinMinPointSeparation->setValue(m_modelPointMatchAfter->minPointSeparation());
  m_spinPointSize->setValue(m_modelPointMatchAfter->maxPointSize());

  int indexAccepted = m_cmbAcceptedPointColor->findData(QVariant(m_modelPointMatchAfter->paletteColorAccepted()));
  Q_ASSERT (indexAccepted >= 0);
  m_cmbAcceptedPointColor->setCurrentIndex(indexAccepted);

  int indexCandidate = m_cmbCandidatePointColor->findData(QVariant(m_modelPointMatchAfter->paletteColorCandidate()));
  Q_ASSERT (indexCandidate >= 0);
  m_cmbCandidatePointColor->setCurrentIndex(indexCandidate);

  int indexRejected = m_cmbRejectedPointColor->findData(QVariant(m_modelPointMatchAfter->paletteColorRejected()));
  Q_ASSERT (indexRejected >= 0);
  m_cmbRejectedPointColor->setCurrentIndex(indexRejected);

  // Fix the preview size using an invisible boundary
  QGraphicsRectItem *boundary = m_scenePreview->addRect (QRect (0,
                                                                0,
                                                                cmdMediator.document().pixmap().width (),
                                                                cmdMediator.document().pixmap().height ()));
  boundary->setVisible (false);

  m_scenePreview->addPixmap (cmdMediator.document().pixmap());

  updateControls();
  enableOk (false); // Disable Ok button since there not yet any changes
  updatePreview();
}

double DlgSettingsPointMatch::radiusAlongDiagonal () const
{
  double minPointSeparation = m_modelPointMatchAfter->minPointSeparation();
  double maxPointSize = m_modelPointMatchAfter->maxPointSize();

  return minPointSeparation + qSqrt (2.0) * maxPointSize / 2.0;
}

void DlgSettingsPointMatch::slotAcceptedPointColor (const QString &)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsPointMatch::slotAcceptedPointColor";

  m_modelPointMatchAfter->setPaletteColorAccepted((ColorPalette) m_cmbAcceptedPointColor->currentData().toInt());

  updateControls();
  updatePreview();
}

void DlgSettingsPointMatch::slotCandidatePointColor (const QString &)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsPointMatch::slotCandidatePointColor";

  m_modelPointMatchAfter->setPaletteColorCandidate((ColorPalette) m_cmbCandidatePointColor->currentData().toInt());
  updateControls();
  updatePreview();
}

void DlgSettingsPointMatch::slotMaxPointSize (int maxPointSize)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsPointMatch::slotMaxPointSize";

  m_modelPointMatchAfter->setMaxPointSize(maxPointSize);
  updateControls();
  updatePreview();
}

void DlgSettingsPointMatch::slotMinPointSeparation (int minPointSeparation)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsPointMatch::slotMinPointSeparation";

  m_modelPointMatchAfter->setMinPointSeparation(minPointSeparation);
  updateControls();
  updatePreview();
}

void DlgSettingsPointMatch::slotMouseMove (QPointF pos)
{
  // Move the box so it follows the mouse move
  if (m_boxSize != 0) {

    // Do not move any part outside the preview window or else ugly, and unwanted, shifting will occur
    if (pos.x() - radiusAlongDiagonal () < 0) {
      pos.setX (radiusAlongDiagonal ());
    }
    if (pos.y() - radiusAlongDiagonal () < 0) {
      pos.setY (radiusAlongDiagonal ());
    }
    if (pos.x() + radiusAlongDiagonal () > m_scenePreview->sceneRect().width ()) {
      pos.setX (m_scenePreview->sceneRect().width() - radiusAlongDiagonal ());
    }
    if (pos.y() + radiusAlongDiagonal () > m_scenePreview->sceneRect().height ()) {
      pos.setY (m_scenePreview->sceneRect().height() - radiusAlongDiagonal ());
    }

    m_boxSize->setPos (pos);
  }
}

void DlgSettingsPointMatch::slotRejectedPointColor (const QString &)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsPointMatch::slotRejectedPointColor";

  m_modelPointMatchAfter->setPaletteColorRejected((ColorPalette) m_cmbRejectedPointColor->currentData().toInt());
  updateControls();
  updatePreview();
}

void DlgSettingsPointMatch::updateControls()
{
  // All controls in this dialog are always fully validated so the ok button is always enabled (after the first change)
  enableOk (true);
}

void DlgSettingsPointMatch::updatePreview()
{
  QPen pen (QBrush (Qt::black), 0);

  if (m_boxSize != 0) {
    m_scenePreview->removeItem (m_boxSize);
    delete m_boxSize;
    m_boxSize = 0;
  }

  // Geometry parameters
  double maxPointSize = m_modelPointMatchAfter->maxPointSize();
  double minPointSeparation = m_modelPointMatchAfter->minPointSeparation();

  double xLeft = -1.0 * maxPointSize / 2.0;
  double xRight = maxPointSize / 2.0;
  double yTop = -1.0 * maxPointSize / 2.0;
  double yBottom = maxPointSize / 2.0;
  double diagonalSide = minPointSeparation / qSqrt (2.0);

  // Create a box in the center
  m_boxSize = new QGraphicsRectItem (xLeft,
                                     yTop,
                                     maxPointSize,
                                     maxPointSize);
  m_boxSize->setPos (cmdMediator().document().pixmap().width () / 2.0,
                     cmdMediator().document().pixmap().height () / 2.0); // Initially box is in center of preview
  m_boxSize->setPen (pen);
  m_boxSize->setZValue (100);
  m_scenePreview->addItem (m_boxSize);

  // Create one diagonal line extending from each corner of the box in the center. They are children to the
  // box so (1) dragging the box causes the lines to get dragged and (2) deleting the box causes the lines to get deleted.
  // These are children so they do not need to be added to the scene
  QGraphicsLineItem *lineTL = new QGraphicsLineItem (xLeft,
                                                     yTop,
                                                     xLeft - diagonalSide,
                                                     yTop - diagonalSide,
                                                     m_boxSize);
  QGraphicsLineItem *lineTR = new QGraphicsLineItem (xRight,
                                                     yTop,
                                                     xRight + diagonalSide,
                                                     yTop - diagonalSide,
                                                     m_boxSize);
  QGraphicsLineItem *lineBL = new QGraphicsLineItem (xLeft,
                                                     yBottom,
                                                     xLeft - diagonalSide,
                                                     yBottom + diagonalSide,
                                                     m_boxSize);
  QGraphicsLineItem *lineBR = new QGraphicsLineItem (xRight,
                                                     yBottom,
                                                     xRight + diagonalSide,
                                                     yBottom + diagonalSide,
                                                     m_boxSize);
  lineTL->setPen (pen);
  lineTR->setPen (pen);
  lineBL->setPen (pen);
  lineBR->setPen (pen);

  // Draw a circle circumscribing the four diagonals. Like the diagonals, this is a child to the box in the center.
  // Since this is a child it does not need to be added to the scene
  QGraphicsEllipseItem *circle = new QGraphicsEllipseItem (-1.0 * radiusAlongDiagonal (),
                                                           -1.0 * radiusAlongDiagonal (),
                                                           2.0 * radiusAlongDiagonal (),
                                                           2.0 * radiusAlongDiagonal (),
                                                           m_boxSize);
  circle->setPen (pen);
}
