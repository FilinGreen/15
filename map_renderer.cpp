#include "map_renderer.h"

MapRenderer::MapRenderer(const json::Node& data) :data_(data) {}

void MapRenderer::Draw(TransportCatalogue& catalog, renderer::RenderSettings& settings, std::ostream& out, renderer::SphereProjector& proj) {
	svg::Document doc;                                             //Документ для печати примитивов

	std::vector<std::string> buses;                                //вектор маршрутов чтоб потом сортировать
	buses.reserve((catalog.GetBuses()).size());

	for (Bus& bus : catalog.GetBuses()) {                          //заполняем вектор маршрутов
		if (catalog.GetRout(bus.name).size() != 0) {
			buses.push_back(bus.name);
		}
	}
	std::sort(buses.begin(), buses.end());                         //сортируем вектор маршрутов


	int max_color = settings.color_palette.size();                 //количество цветов
	int bus_number = max_color;                                    //начальная переменная для цвета

	for (std::string& busname : buses) {                           //проходим по маршрутам и отрисовка линий для маршрутов с остановками
		DrawLine(doc, settings, proj, bus_number % max_color, catalog.GetRout(busname));
		++bus_number;
	}

	bus_number = max_color;                                        //начальная переменная для цвета

	for (std::string& busname : buses) {                           //проходим по маршрутам и отрисовка названий для маршрутов с остановками
		DrawBusname(catalog, busname, doc, settings, proj, bus_number % max_color);
		++bus_number;
	}

	std::vector<std::string> stops;                                 //вектор остановок чтобы потом сортировать
	stops.reserve((catalog.GetStops()).size());

	for (Stop& stop : catalog.GetStops()) {
		if (catalog.HasBusAtStop(stop.name)) {
			stops.push_back(stop.name);
		}
	}

	std::sort(stops.begin(), stops.end());

	for (std::string& stopname : stops) {                              //отрисовка остановок
		DrawStops(doc, { (catalog.GetStop(stopname)).x,(catalog.GetStop(stopname)).y }, proj, settings);
	}

	for (std::string& stopname : stops) {                              //отрисовка названий остановок
		DrawStopName(doc, { (catalog.GetStop(stopname)).x,(catalog.GetStop(stopname)).y }, settings, proj, stopname);
	}

	doc.Render(out);
}

void MapRenderer::DrawLine(svg::Document& doc, renderer::RenderSettings& settings, renderer::SphereProjector& proj, int color, const std::vector<Stop*>& route) {

	svg::Polyline line;                                //создаем линию и заполняем данные
	line.SetFillColor("none");
	line.SetStrokeWidth(settings.line_width);
	line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
	line.SetStrokeColor(settings.color_palette.at(color));

	std::vector<geo::Coordinates> coordinates;         //вектор координат остановок маршрута
	for (size_t i = 0; i < route.size(); ++i) {
		coordinates.push_back({ route.at(i)->x, route.at(i)->y });
	}

	for (auto& point : coordinates) {                   //проходим по вектору и проецируем все координаты добавляя их в линию
		svg::Point p = proj(point);
		line.AddPoint({ p.x, p.y });
	}

	doc.Add(line);                                       //добавляем примитив в документ примитивов
}

void MapRenderer::DrawBusname(TransportCatalogue& catalog, const std::string& busname, svg::Document& doc, renderer::RenderSettings& settings, renderer::SphereProjector& proj, int color) { // Отрисовка названий маршрута
	Bus bus = catalog.GetBus(busname);

	geo::Coordinates bus_coor = { catalog.GetRout(bus.name).at(0)->x , catalog.GetRout(bus.name).at(0)->y };
	svg::Point point = proj(bus_coor);
	svg::Text under;
	svg::Text upper;
	under.SetPosition({ point.x, point.y });
	upper.SetPosition({ point.x, point.y });
	under.SetOffset(settings.bus_label_offset);
	upper.SetOffset(settings.bus_label_offset);
	under.SetFontSize(settings.bus_label_font_size);
	upper.SetFontSize(settings.bus_label_font_size);
	under.SetFontFamily("Verdana");
	upper.SetFontFamily("Verdana");
	under.SetFontWeight("bold");
	upper.SetFontWeight("bold");
	under.SetData(busname);
	upper.SetData(busname);
	under.SetFillColor(settings.underlayer_color);
	under.SetStrokeColor(settings.underlayer_color);
	upper.SetFillColor(settings.color_palette.at(color));
	under.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
	under.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	under.SetStrokeWidth(settings.underlayer_width);

	doc.Add(under);
	doc.Add(upper);

	if (!bus.circle && catalog.GetRout(bus.name).at(0) != catalog.GetRout(bus.name).at((catalog.GetRout(bus.name).size() - 1) / 2)) { //Проверка что маршрут не кольцевой и конечные остановки не совпадают

		size_t i = (catalog.GetRout(bus.name).size() - 1) / 2;

		geo::Coordinates bus_coor2 = { catalog.GetRout(bus.name).at(i)->x , catalog.GetRout(bus.name).at(i)->y };
		svg::Point point2 = proj(bus_coor2);
		svg::Text under2;
		svg::Text upper2;
		under2.SetPosition({ point2.x, point2.y });
		upper2.SetPosition({ point2.x, point2.y });
		under2.SetOffset(settings.bus_label_offset);
		upper2.SetOffset(settings.bus_label_offset);
		under2.SetFontSize(settings.bus_label_font_size);
		upper2.SetFontSize(settings.bus_label_font_size);
		under2.SetFontFamily("Verdana");
		upper2.SetFontFamily("Verdana");
		under2.SetFontWeight("bold");
		upper2.SetFontWeight("bold");
		under2.SetData(busname);
		upper2.SetData(busname);
		under2.SetFillColor(settings.underlayer_color);
		under2.SetStrokeColor(settings.underlayer_color);
		upper2.SetFillColor(settings.color_palette.at(color));
		under2.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		under2.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		under2.SetStrokeWidth(settings.underlayer_width);
		doc.Add(under2);
		doc.Add(upper2);
	}
}

void MapRenderer::DrawStops(svg::Document& doc, const geo::Coordinates& coordinates, renderer::SphereProjector& proj, renderer::RenderSettings& settings) { // Отрисовка графического отображения остановки

	svg::Circle circle;
	svg::Point point = proj(coordinates);
	circle.SetCenter(point);
	circle.SetRadius(settings.stop_radius);
	circle.SetFillColor("white");
	doc.Add(circle);

}

void MapRenderer::DrawStopName(svg::Document& doc, const geo::Coordinates& coordinates, renderer::RenderSettings& settings, renderer::SphereProjector& proj, const std::string& stopname) { // Отрисовка названия остановки

	svg::Point point = proj(coordinates);
	svg::Text under;
	svg::Text upper;
	under.SetPosition({ point.x, point.y });
	upper.SetPosition({ point.x, point.y });
	under.SetOffset(settings.stop_label_offset);
	upper.SetOffset(settings.stop_label_offset);
	under.SetFontSize(settings.stop_label_font_size);
	upper.SetFontSize(settings.stop_label_font_size);
	under.SetFontFamily("Verdana");
	upper.SetFontFamily("Verdana");
	under.SetData(stopname);
	upper.SetData(stopname);
	under.SetFillColor(settings.underlayer_color);
	under.SetStrokeColor(settings.underlayer_color);
	upper.SetFillColor("black");
	under.SetStrokeWidth(settings.underlayer_width);
	under.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
	under.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

	doc.Add(under);
	doc.Add(upper);

}