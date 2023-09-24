#include "json_reader.h"

JSONReader::JSONReader(const json::Node& data) :
	data_(data) {}


void JSONReader::LoadSettings() {
	if (data_.AsDict().count("base_requests") != 0) {
		base_data_ = (data_.AsDict()).at("base_requests");
	}

	if (data_.AsDict().count("routing_settings") != 0) {
		setting_data_ = (data_.AsDict()).at("routing_settings");

		if (data_.AsDict().count("routing_settings") != 0) {
			bus_wait_time_ = setting_data_.AsDict().at("bus_wait_time").AsInt();
			bus_velocity_ = setting_data_.AsDict().at("bus_velocity").AsDouble();
		}
		else {
			bus_wait_time_ = 0;
			bus_velocity_ = 0;
		}
	}
	
	if (data_.AsDict().count("serialization_settings") != 0) {
		serialization_data_ = (data_.AsDict()).at("serialization_settings");
	}

	if (data_.AsDict().count("render_settings") != 0) {
		settings_ = LoadRenderSettings();
	}
	

}


void JSONReader::LoadData() {
	LoadSettings();
	LoadStops();
	LoadBuses();
}

void JSONReader::Serialization() {	
	TransportRouter router(catalog_, bus_wait_time_, bus_velocity_);
	srlz::Serialization(catalog_, router, settings_, serialization_data_.AsDict().at("file").AsString());
}


void JSONReader::Deserialization() {
	LoadSettings();
	srlz::Deserialization(catalog_, settings_, bus_wait_time_, bus_velocity_, serialization_data_.AsDict().at("file").AsString());
}


void JSONReader::ProcessQuery(std::ostream& out) {
	json::Node requests_data = (data_.AsDict()).at("stat_requests");
	TransportRouter transport_router_(catalog_, bus_wait_time_, bus_velocity_);
	

	json::Builder builder {};                                    //Обработчик вывода ответов
	builder.StartArray();

	for (json::Node request : requests_data.AsArray()) {         // Проходим по всем запросам

		if (((request.AsDict()).at("type")).AsString() == "Bus") {
			ProcessBus(builder, request);
		}
		else if (((request.AsDict()).at("type")).AsString() == "Stop") {
			ProcessStop(builder, request);
		}
		else if (((request.AsDict()).at("type")).AsString() == "Route") {
			ProcessRoute(builder, request, transport_router_);
		}
		else {
			ProcessMap(builder, request);
		}
                                                  
	}//for
	builder.EndArray();

	json::Print(json::Document{std::move(builder.Build())}, out);                                 // Выводим ответы в поток вывода out
	
	
}

void JSONReader::LoadStops() {
	for (json::Node stop_data : base_data_.AsArray()) {                                                      //Пробегаемся по контейнеру заполнения базы
		auto mapa = stop_data.AsDict();
		if (((mapa.at("type")).AsString()) == "Stop") {                                                      //Проверяем что элемент является остановкой
			std::string stopname = (mapa.at("name")).AsString();                                             //Имя остановки
			catalog_.AddStop(stopname, (mapa.at("latitude")).AsDouble(), (mapa.at("longitude")).AsDouble()); //Добавляем остановку
			for (auto [name, distance] : (mapa.at("road_distances")).AsDict()) {                             //Добавляем дистанции
				catalog_.AddDistance(stopname, name, distance.AsDouble());
			}
		}
	}
}

void JSONReader::LoadBuses() {

	for (json::Node stop_data : base_data_.AsArray()) {                 //Пробегаем по контейнеру заполнения базы
		auto mapa = stop_data.AsDict();
		if (((mapa.at("type")).AsString()) == "Bus") {                  //Проверяем что элемент является маршрутом/автобусом
			std::vector<std::string> bus_stops;
			bus_stops.reserve(((mapa.at("stops")).AsArray()).size());

			for (json::Node stop : ((mapa.at("stops")).AsArray())) {    //Заполняем контейнер остановок
				bus_stops.push_back(stop.AsString());
			}

			if (!(mapa.at("is_roundtrip")).AsBool()) {                  //Если маршрут не кольцевой то добавляем остановки в обратную сторону

				bus_stops.reserve(((mapa.at("stops")).AsArray()).size() * 2);
				size_t z = bus_stops.size() - 2;

				for (size_t i = z; i + 1 > 0; --i) {
					bus_stops.push_back(bus_stops.at(i));
				}//for

			}//if

			catalog_.AddBus((mapa.at("name")).AsString(), bus_stops, (mapa.at("is_roundtrip")).AsBool(), bus_wait_time_, ((bus_velocity_ * 1000.0) / 60.0));   //Добавляем маршрут/автобус
		}
	}
}

void JSONReader::ProcessBus(json::Builder& builder, const json::Node& request) {
	std::string bus_name = ((request.AsDict()).at("name")).AsString();

	builder.StartDict();

	if (!catalog_.HasBus(bus_name)) {                               // Проверка наличия маршрута в каталоге

		builder.Key("request_id").Value((request.AsDict()).at("id").AsInt());
		builder.Key("error_message").Value("not found");
		builder.EndDict();
		
		return;
	}

	double geo_distance = 0.0;                                      // Общая географическая дистанция
	int route_lenght = 0;                                           // Общая реальная дистанция
	std::unordered_set <std::string_view> unique_stops;             // Сет уникальных остановок
	Stop* prev_stop = catalog_.GetRout(bus_name).at(0);             // Предыдущая остановка
	bool flag = true;

	for (auto& stop : catalog_.GetRout(bus_name)) {                 // Обработка маршрута
		unique_stops.insert((*stop).name);
		if (flag) {
			flag = false;

			continue;
		}
		geo_distance += geo::ComputeDistance({ (*prev_stop).x,(*prev_stop).y }, { (*stop).x,(*stop).y });
		route_lenght += catalog_.GetDistance(prev_stop, stop);
		prev_stop = stop;

	}//for

	//Добавление данных в файл ответа

	
	builder.Key("request_id").Value((request.AsDict()).at("id").AsInt());
	builder.Key("stop_count").Value(static_cast<int>((catalog_.GetRout(bus_name)).size()));
	builder.Key("curvature").Value(route_lenght / geo_distance);
	builder.Key("route_length").Value(route_lenght);
	builder.Key("unique_stop_count").Value(static_cast<int>(unique_stops.size()));

	builder.EndDict();
}

void JSONReader::ProcessStop(json::Builder& builder, const json::Node& request) {
	std::string stop_name = ((request.AsDict()).at("name")).AsString(); // Имя остановки

	builder.StartDict();

	if (!catalog_.HasStop(stop_name)) {                                 // Проверка что остановка существует

		builder.Key("request_id").Value((request.AsDict()).at("id").AsInt());
		builder.Key("error_message").Value("not found");

		builder.EndDict();
		return;
	}

	std::set<std::string_view> buses = catalog_.GetBuses(stop_name);    // Получение списка маршрутов остановки из каталога
	std::vector<json::Node> sbuses;
	sbuses.reserve(buses.size());

	for (std::string_view bus : buses) {
		sbuses.push_back(std::string(bus));
	}                                         

	builder.Key("request_id").Value((request.AsDict()).at("id").AsInt());// Добавление id запроса в файл ответа
	builder.Key("buses").Value(sbuses);                                  // Добавлениеконтейнера маршрутов в файл ответа

	builder.EndDict();
}

void JSONReader::ProcessRoute(json::Builder& builder, const json::Node& request, TransportRouter& transport_router) {

	size_t from = catalog_.GetStop(((request.AsDict()).at("from")).AsString()).stop_id; //stop_id это id входной вершины
	size_t to = catalog_.GetStop(((request.AsDict()).at("to")).AsString()).stop_id; //stop_id+1 это id выходной вершины

	transport_router.ProcessRoute(builder, request, from, to);
}

renderer::SphereProjector JSONReader::SetProjector() { // Задаем класс смещения
	std::vector<geo::Coordinates> allcoor;
	for (auto& stop : catalog_.GetStops()) {
		if (catalog_.GetBuses(stop.name).size() != 0) {
			allcoor.push_back({ stop.x, stop.y });
		}
	}
	renderer::SphereProjector proj{ allcoor.begin(), allcoor.end(), settings_.size.x, settings_.size.y, settings_.padding};
	return proj;
}

renderer::RenderSettings JSONReader::LoadRenderSettings() { // Загрузка настроек отображения карты
	json::Node graphic_data = (data_.AsDict()).at("render_settings");

	renderer::RenderSettings settings;
	std::map<std::string, json::Node> sets = graphic_data.AsDict();
	settings.size.x = (sets.at("width")).AsDouble();
	settings.size.y = (sets.at("height")).AsDouble();
	settings.padding = (sets.at("padding")).AsDouble();
	settings.line_width = (sets.at("line_width")).AsDouble();
	settings.stop_radius = (sets.at("stop_radius")).AsDouble();
	settings.bus_label_font_size = (sets.at("bus_label_font_size")).AsInt();
	settings.bus_label_offset = LoadOffset((sets.at("bus_label_offset")).AsArray());
	settings.stop_label_font_size = (sets.at("stop_label_font_size")).AsInt();
	settings.stop_label_offset = LoadOffset((sets.at("stop_label_offset")).AsArray());
	settings.underlayer_color = LoadColor(sets.at("underlayer_color"));
	settings.underlayer_width = (sets.at("underlayer_width")).AsDouble();

	std::vector<svg::Color> colors;
	colors.reserve(sets.at("color_palette").AsArray().size());
	for (json::Node color : sets.at("color_palette").AsArray()) {
		colors.push_back(LoadColor(color));
	}
	settings.color_palette = colors;

	return settings;
}

svg::Color JSONReader::LoadColor(const json::Node& data) { // Загрузка Color из json
	if (data.IsString()) {
		return data.AsString();
	}
	else if (data.IsArray()) {
		if ((data.AsArray()).size() == 3) {
			return svg::Rgb(
				(data.AsArray()).at(0).AsInt(),
				(data.AsArray()).at(1).AsInt(),
				(data.AsArray()).at(2).AsInt());
		}
		else {
			return svg::Rgba(
				(data.AsArray()).at(0).AsInt(),
				(data.AsArray()).at(1).AsInt(),
				(data.AsArray()).at(2).AsInt(),
				(data.AsArray()).at(3).AsDouble());
		}
	}
	return svg::NoneColor;
}

svg::Point JSONReader::LoadOffset(const std::vector<json::Node>& data) { //Загрузка Point из json
	svg::Point result;
	if (data.size() > 1) {
		result.x = data.at(0).AsDouble();
		result.y = data.at(1).AsDouble();
	}
	return result;
}

void JSONReader::ProcessMap(json::Builder& builder, const json::Node& request) {
	
	renderer::SphereProjector proj = SetProjector();

	std::ostringstream sout;                                             // Определение потока вывода
	MapRenderer maprender(data_);
	maprender.Draw(catalog_, settings_, sout, proj);                     // Отрисовка карты и вывод svg файла в поток вывода          
   

	builder.StartDict();

	builder.Key("map").Value(sout.str());                                // Добавление карты в файл ответа
	builder.Key("request_id").Value((request.AsDict()).at("id").AsInt());// Добавление id запроса в файл ответа

	builder.EndDict();
}


