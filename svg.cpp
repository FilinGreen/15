#include "svg.h"

namespace svg {

    using namespace std::literals;

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& value) {
        switch (value) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;

        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;

        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
        }
        return  out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& value) {
        switch (value) {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;

        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;

        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;

        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;

        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
        }
        return  out;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }


    // ---------- Polyline ------------------    

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }
    //<polyline points="0,100 50,25 50,75 100,0" />    
    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;

        out << "<polyline points=\"";

        bool space = false;
        for (auto& point : points_) {

            if (!space) {
                out << point.x << "," << point.y;
                space = true;
            }
            else {
                out << " " << point.x << "," << point.y;
            }
        }
        out << "\"";
        RenderAttrs(out);
        out << "/>"sv;
    }


    // ---------- Text ------------------ 

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }


    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }


    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;

        out << "<text";
        const Point zero;

        RenderAttrs(out);

        out << " x=\"" << pos_.x << "\" y=\"" << pos_.y << "\"";
        out << " dx=\"" << offset_.x << "\" dy=\"" << offset_.y << "\"";
        out << " font-size=\"" << size_ << "\"";


        if (!font_family_.empty()) {
            out << " font-family=\"" << font_family_ << "\"";
        }

        if (!font_weight_.empty()) {
            out << " font-weight=\"" << font_weight_ << "\"";
        }

        out << ">";

        std::string text;
        if (!data_.empty()) {
            for (const char& lit : data_) {
                switch (lit)
                {
                case '\"':
                    text.append("&quot;");
                    break;

                case '\'':
                    text.append("&apos;");
                    break;

                case '<':
                    text.append("&lt;");
                    break;

                case '>':
                    text.append("&gt;");
                    break;

                case '&':
                    text.append("&amp;");
                    break;

                default:
                    text.push_back(lit);
                }//witch
            }//for
            out << text;
        }


        out << "</text>";
    }//TextRender    

    // ---------- Document ------------------   

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        RenderContext context(out);

        context.out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";
        context.out << "\n";
        context.out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">";
        context.out << "\n";

        for (const auto& obj : objects_) {
            obj->Render(context);
        }

        context.out << "</svg>";
    }

}  // namespace svg