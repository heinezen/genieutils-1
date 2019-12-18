#include "genie/resource/SmpFrame.h"
namespace genie {

Logger &SmpFrame::log = Logger::getLogger("genie.SmpFrame");
SmpFrame SmpFrame::null;

void SmpFrame::serializeObject()
{
    assert(false && "not implemented");

    serialize<uint32_t>(width_);
    serialize<uint32_t>(height_);

    serialize<int32_t>(hotspot_x);
    serialize<int32_t>(hotspot_y);

    serialize(m_layerType);

    serialize(outline_table_offset_);
    serialize(cmd_table_offset_);

    // Flags, but we don't know exactly what they do
    // 0x01, 0x02, 0x80 or 0xA0 are example values from heinezen
    serialize(flags_);

    //----------------------------------------------------------------------------
    /// Reads the edges of the frame. An edge int is the number of pixels in
    /// a row which are transparent. There are two 16 bit unsigned integers for
    /// each side of a row. One starting from left and the other starting from the
    /// right side.
    getIStream()->seekg(smp_file_pos_ + std::streampos(outline_table_offset_));

    left_edges_.resize(height_, 0);
    right_edges_.resize(height_, 0);

    for (uint32_t row = 0; row < height_; ++row) {
        serialize<uint16_t>(left_edges_[row]);
        serialize<uint16_t>(right_edges_[row]);
    }
}

void SmpFrame::readImage()
{
    std::istream &istr = *getIStream();

    const size_t byteCount = width_ * height_;
    smp_pixels.resize(byteCount);

    size_t pixelsRead = 0;

    // Each row has it's commands, 0x03 signals the end of a rows commands.
    for (uint32_t row = 0; row < height_; ++row) {
//        istr.seekg(slp_file_pos_ + std::streampos(cmd_offsets_[row]));
//        std::cout << "command offset: " << cmd_offsets_[row] << std::endl;
//        std::cout << "file pos: " << slp_file_pos_ << std::endl;
        assert(!istr.eof());

        // Transparent rows apparently read one byte anyway. NO THEY DO NOT! Ignore and use seekg()
        if (0xFFFF == left_edges_[row] || 0xFFFF == right_edges_[row]) { // Remember signedness!
            continue; // Pretend it does not exist.
        }

        uint32_t pix_pos = left_edges_[row]; //pos where to start putting pixels

        while (true) {
            const uint8_t data = read<uint8_t>();
            if (data == 3) { // end of row
                break;
            }

            const uint32_t pix_cnt = (data >> 2) + 1;

            switch(data & 0b11) {
            case 0: // Skip
                break;
            case 1: // Normal colors
                readSmpPixelstoImage(row, pix_pos, pix_cnt, false);
                break;
            case 2: // Player colors
                readSmpPixelstoImage(row, pix_pos, pix_cnt, true);
                break;
            }

            pix_pos += pix_cnt;
            pixelsRead += pix_cnt;

        }
    }

    if (pixelsRead == 0) {
        width_ = 0;
        height_ = 0;
    }

}

void SmpFrame::readSmpPixelstoImage(uint32_t row, uint32_t &col, uint32_t count, bool player_col)
{
    if (!player_col) {
        getIStream()->read((char *)&smp_pixels.data()[row * width_ + col], count * sizeof(SmpPixel));
        col += count;
        return;
    }

    std::vector<char> bgras(count);
    getIStream()->read(bgras.data(), count);

    uint32_t to_pos = col + count;
    while (col < to_pos) {
        SmpPixel pixel;
        getIStream()->read((char *)&pixel, sizeof(SmpPixel));
        smp_player_color_mask.push_back({ col, row, pixel });

        ++col;
    }
}

}//namespace genie
