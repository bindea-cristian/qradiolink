// Written by Adrian Musceac YO8RZZ , started March 2016.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "imagecapture.h"
#include <QElapsedTimer>
#include <QCameraImageCapture>
//#define FRAME_SIZE 230400 // 320 x 240 x 3 (RGB)
#define FRAME_SIZE 56700 // 160 x 120 x 3

ImageCapture::ImageCapture(Settings *settings, Logger *logger, QObject *parent) : QObject(parent)
{
    _settings = settings;
    _logger = logger;
    _inited = false;
    _shutdown = false;
    _capturing = false;
    _last_frame_length = 0;
    _videobuffer = new unsigned char[FRAME_SIZE];
    memset(_videobuffer, 0, FRAME_SIZE*sizeof(unsigned char));
}

ImageCapture::~ImageCapture()
{
    deinit();
    delete[] _videobuffer;
}

void ImageCapture::init()
{
    _mutex.lock();
    if(_inited)
    {
        _mutex.unlock();
        return;
    }
    _mutex.unlock();

    // SIMULATION MODE - Load test image instead of using camera
    QString simulation_path = "src/res/test_frame.jpg";
    if (!_simulation_image.load(simulation_path))
    {
        _logger->log(Logger::LogLevelCritical, QString("Failed to load simulation image: %1").arg(simulation_path));
        return;
    }

    // Ensure the image is the correct size and format
    _simulation_image = _simulation_image.scaled(160, 120, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    _simulation_image = _simulation_image.convertToFormat(QImage::Format_RGB888);

    // Prepare the video buffer with the simulation image
    unsigned char *data = (unsigned char*)_simulation_image.bits();
    _last_frame_length = _simulation_image.sizeInBytes();
    memcpy(_videobuffer, data, _last_frame_length);

    _logger->log(Logger::LogLevelInfo, QString("Camera simulation initialized with image size: %1 bytes").arg(_last_frame_length));

    _mutex.lock();
    _inited = true;
    _mutex.unlock();
}

void ImageCapture::process_img_error(int id, QCameraImageCapture::Error error, const QString &errorString)
{
    _logger->log(Logger::LogLevelCritical, "Capture img ERROR:  " + errorString);
}

void ImageCapture::deinit()
{
    _mutex.lock();
    if(!_inited)
    {
        _mutex.unlock();
        return;
    }

    // SIMULATION MODE - Simple cleanup
    _shutdown = true;
    _simulation_image = QImage(); // Clear the simulation image

    _inited = false;
    _shutdown = false;
    _mutex.unlock();
}

void ImageCapture::capture_image()
{
    _logger->log(Logger::LogLevelInfo, "6 ====== Start capture_image (simulation)");
    _mutex.lock();
    if((!_inited) || (_shutdown))
    {
        _logger->log(Logger::LogLevelCritical, "6 ====== CLOSING CAPTURE IMAGE....");
        _mutex.unlock();
        return;
    }

    // SIMULATION MODE - No actual capture needed, image is already loaded
    _logger->log(Logger::LogLevelInfo, "6 ====== CAPTURE IMAGE (simulation) - returning pre-loaded frame");
    _mutex.unlock();
}

void ImageCapture::process_image_available(int id, const QVideoFrame &frame)
{
    _logger->log(Logger::LogLevelInfo, QString("7 ======= Image AVAILABLE, WITH ID: %1").arg(id));
}

void ImageCapture::process_image(int id, QImage img)
{
//    Q_UNUSED(id);
    _logger->log(Logger::LogLevelInfo, QString("8 ======== Image captured, now processing %1").arg(id));
    img = img.convertToFormat(QImage::Format_RGB888);
    unsigned char *data = (unsigned char*)img.bits();
    _last_frame_length = img.sizeInBytes();
    memcpy(_videobuffer, data, _last_frame_length);
    _logger->log(Logger::LogLevelInfo, QString("8 ======== Image captured, done processing lastframelen  %1 ").arg(_last_frame_length));
}

unsigned char* ImageCapture::get_frame(int &len)
{
    _logger->log(Logger::LogLevelInfo, "5 ===== Start get_frame (simulation)");
    if(!_inited)
    {
        len = 0;
        _logger->log(Logger::LogLevelCritical, "5 ===== Get frame error - not initialized");
        return nullptr;
    }

    // SIMULATION MODE - Return pre-loaded frame data directly
    len = _last_frame_length;
    if(len == 0){
        _logger->log(Logger::LogLevelCritical, "5 ===== Get frame error - no simulation data");
        return nullptr;
    }
    unsigned char* frame = new unsigned char[FRAME_SIZE];
    memcpy(frame, _videobuffer, FRAME_SIZE*sizeof(unsigned char));
    _logger->log(Logger::LogLevelInfo, "5 ===== End get_frame (simulation) - returning same frame");
    return frame;
}
