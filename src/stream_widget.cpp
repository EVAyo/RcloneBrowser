#include "stream_widget.h"

StreamWidget::StreamWidget(QProcess *rclone, QProcess *player,
                           const QString &remote, const QString &stream,
                           QWidget *parent)
    : QWidget(parent), mRclone(rclone), mPlayer(player) {
  ui.setupUi(this);

  QString remoteTrimmed;

  if (remote.length() > 140) {
    remoteTrimmed = remote.left(57) + "..." + remote.right(80);
  } else {
    remoteTrimmed = remote;
  }

  ui.info->setText(remoteTrimmed);
  ui.info->setCursorPosition(0);

  ui.stream->setText(stream);
  ui.stream->setCursorPosition(0);
  ui.stream->setToolTip(stream);

  ui.remote->setText(remote);
  ui.remote->setCursorPosition(0);
  ui.remote->setToolTip(remote);

  ui.details->setVisible(false);

  ui.output->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  ui.output->setVisible(false);

  ui.showDetails->setIcon(
      QIcon(":remotes/images/qbutton_icons/vrightarrow.png"));
  ui.showDetails->setIconSize(QSize(24, 24));

  ui.showOutput->setIcon(
      QIcon(":remotes/images/qbutton_icons/vrightarrow.png"));
  ui.showOutput->setIconSize(QSize(24, 24));

  ui.cancel->setToolTip("Stop streaming");

  QObject::connect(
      ui.showDetails, &QToolButton::toggled, this, [=](bool checked) {
        ui.details->setVisible(checked);
        if (checked) {
          ui.showDetails->setIcon(
              QIcon(":remotes/images/qbutton_icons/vdownarrow.png"));
          ui.showDetails->setIconSize(QSize(24, 24));
        } else {
          ui.showDetails->setIcon(
              QIcon(":remotes/images/qbutton_icons/vrightarrow.png"));
          ui.showDetails->setIconSize(QSize(24, 24));
        }
      });

  QObject::connect(
      ui.showOutput, &QToolButton::toggled, this, [=](bool checked) {
        ui.output->setVisible(checked);
        if (checked) {
          ui.showOutput->setIcon(
              QIcon(":remotes/images/qbutton_icons/vdownarrow.png"));
          ui.showOutput->setIconSize(QSize(24, 24));
        } else {
          ui.showOutput->setIcon(
              QIcon(":remotes/images/qbutton_icons/vrightarrow.png"));
          ui.showOutput->setIconSize(QSize(24, 24));
        }
      });

  ui.cancel->setIcon(QIcon(":remotes/images/qbutton_icons/cancel.png"));
  ui.cancel->setIconSize(QSize(24, 24));

  QObject::connect(ui.cancel, &QToolButton::clicked, this, [=]() {
    if (mRunning) {
      int button = QMessageBox::question(
          this, "Stop", QString("Do you want to stop %1 stream?").arg(remote),
          QMessageBox::Yes | QMessageBox::No);
      if (button == QMessageBox::Yes) {
        cancel();
      }
    } else {
      emit closed();
    }
  });

  QObject::connect(mRclone, &QProcess::readyRead, this, [=]() {
    while (mRclone->canReadLine()) {
      ui.output->appendPlainText(mRclone->readLine().trimmed());
    }
  });

  QObject::connect(mRclone,
                   static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
                       &QProcess::finished),
                   this, [=](int status, QProcess::ExitStatus) {
                     mRclone->deleteLater();
                     mRunning = false;

                     QString info = "Streaming " + ui.info->text();
                     QString infoTrimmed;
                     if (info.length() > 140) {
                       infoTrimmed = info.left(57) + "..." + info.right(80);
                     } else {
                       infoTrimmed = info;
                     }
                     ui.info->setText(infoTrimmed);
                     ui.info->setCursorPosition(0);

                     if (status == 0 || status == 9) {
                       ui.showDetails->setStyleSheet(
                           "QToolButton { border: 0; color: black; }");
                       ui.showDetails->setText("  Finished");
                     } else {
                       ui.showDetails->setStyleSheet(
                           "QToolButton { border: 0; color: red; }");
                       ui.showDetails->setText("  Error");
                     }

                     ui.cancel->setToolTip("Close");

                     emit finished();
                     //          emit closed();
                   });

  ui.showDetails->setStyleSheet("QToolButton { border: 0; color: green; }");
  ui.showDetails->setText("  Streaming");
}

StreamWidget::~StreamWidget() {}

void StreamWidget::cancel() {
  if (!mRunning) {
    return;
  }

  mPlayer->terminate();
  mRclone->kill();
  mRclone->waitForFinished();
}
