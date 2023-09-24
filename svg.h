#pragma once

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg {

    struct Rgb {
        Rgb() = default;
        Rgb(uint8_t redi, uint8_t greeni, uint8_t bluei)
            :red(redi)
            , green(greeni)
            , blue(bluei) {}

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        Rgba() = default;
        Rgba(uint8_t redi, uint8_t greeni, uint8_t bluei, double opacityi)
            :red(redi)
            , green(greeni)
            , blue(bluei)
            , opacity(opacityi) {}

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    inline const Color NoneColor{"none"};

    struct ColorPrinter {
        std::ostream& out;
        void operator() (std::monostate) const {
            out << "none";
        }

        void operator() (const std::string color) const {
            out << color;
        }

        void operator() (Rgb rgb) const {
            out << "rgb(" << std::to_string(rgb.red) << "," << std::to_string(rgb.green) << "," << std::to_string(rgb.blue) << ")";

        }

        void operator () (Rgba rgba) const {
            out << "rgba(" << std::to_string(rgba.red) << "," << std::to_string(rgba.green) << "," << std::to_string(rgba.blue) << "," << rgba.opacity << ")";
        }

    };


    inline std::ostream& operator<<(std::ostream& out, const Color color) {
        std::visit(ColorPrinter{ out }, color);
        return out;
    }


    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& value);

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };
    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& value);

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object { //класс который объединяет шаблоном остальные фигуры
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {// Цвет заливки по умолчанию не выводится
            fillcolor_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeColor(Color color) {// Цвет контура по умолчанию не выводится
            strokecolor_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {// Толщина линии по умолчанию не выводится
            width_ = std::move(width);
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {// Тип формы конца линии по умолчанию не выводится
            linecap_ = std::move(line_cap);
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {// Тип формы соединения линий по умолчанию не выводится
            linejoin_ = std::move(line_join);
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        std::optional<Color> fillcolor_;
        std::optional<Color> strokecolor_;
        std::optional<double> width_;
        std::optional<StrokeLineCap> linecap_;
        std::optional<StrokeLineJoin> linejoin_;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fillcolor_) {
                out << " fill=\""sv << *fillcolor_ << "\"";
            }

            if (strokecolor_) {
                out << " stroke=\""sv << *strokecolor_ << "\"";
            }

            if (width_) {
                out << " stroke-width=\""sv << *width_ << "\"";
            }

            if (linecap_) {
                out << " stroke-linecap=\""sv << *linecap_ << "\"";
            }

            if (linejoin_) {
                out << " stroke-linejoin=\""sv << *linejoin_ << "\"";
            }

        }//Render




    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }

    };




    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

        /*
         * Прочие методы и данные, необходимые для реализации элемента <polyline>
         */
    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_;
    };

    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text : public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point pos_;
        Point offset_;
        uint32_t size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
    };


    class ObjectContainer { // класс который определяет интерфейс для хранения объектов
    public:

        template<typename Obj>
        void Add(Obj obj) {
            AddPtr(std::make_unique<Obj>(std::move(obj)));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    protected:
        ~ObjectContainer() = default;

        std::vector<std::unique_ptr<Object>> objects_;


    };

    class Drawable { // унифицирует работу с объектами которые будут рисоваться через примитивы
    public:
        virtual ~Drawable() = default;
        virtual void Draw(ObjectContainer& container) const = 0;

    };


    class Document : public ObjectContainer {//Класс который хранит вектор примитивов которые нужно нарисовать.
    public:

        void AddPtr(std::unique_ptr<Object>&& obj); //virtual

        void Render(std::ostream& out) const;

    };

}  // namespace svg