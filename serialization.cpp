#include "serialization.h"

#include <iostream>

using namespace srlz;

void srlz::Serialization(TransportCatalogue& catalog, TransportRouter& transport_router, renderer::RenderSettings& settings, std::string out){
	std::ofstream fout(out, std::ios::binary);                                       //��������� ������ �� ������� � �����
	if(!fout){
		std::cout << "file creating error while serialization" << std::endl;
	}
	
	Protobuf::All_data all_data;                                                     //������� ����� ������ ��� ����������� ������������

	*(all_data.mutable_transport_catalogue()) = ConvertCatalog(catalog);             //���������� ����������������� ������������ �������
	*(all_data.mutable_render_settings()) = ConvertSettings(catalog, settings);      //���������� ����������������� ��������� �������
	*(all_data.mutable_graph_edges()) = ConvertEdges(transport_router);              //���������� ����������������� ������ ����� 
	*(all_data.mutable_router_settings()) = ConvertRouterSettings(transport_router); //���������� ����������������� ��������� �������
	all_data.SerializeToOstream(&fout);                                              //����������� ������
}

void srlz::Deserialization(TransportCatalogue& catalog, renderer::RenderSettings& settings, int& bus_wait_time, double& bus_velocity, std::string in) {
	std::ifstream fin(in, std::ios::binary);                               //��������� ������ �� ������� � �����
	if (!fin) {
		std::cout << "file open error while deserialization" << std::endl;
	}

	Protobuf::All_data all_data;                                           //������� ����� ������
	all_data.ParseFromIstream(&fin);                                       //������������� ���� � ����� ������ � ������������ ������

	catalog = ConvertCatalogFromProto(*all_data.mutable_transport_catalogue());
	settings = ConvertSettingsFromProto(*all_data.mutable_render_settings());
	bus_wait_time = all_data.mutable_router_settings()->bus_wait_tima();
	bus_velocity = all_data.mutable_router_settings()->bus_velocity();
}

//Serialization 
Protobuf::TransportData srlz::ConvertCatalog(TransportCatalogue& catalog) {
	Protobuf::TransportData proto_catalog;                        //������� ����� ������ ������������� ��������

	for (const auto& stop : catalog.GetStops()) {                 //�������� �� ���������� ��������

		Protobuf::Stop* proto_stop = proto_catalog.add_stops();   //������� ����� ������ ��������� � ��������� ���

		proto_stop->set_name(stop.name); 
		proto_stop->set_x(stop.x);
		proto_stop->set_y(stop.y);
		proto_stop->set_stop_id(stop.stop_id);
		
	}

	for (const auto& bus : catalog.GetBuses()) {                  //�������� �� ��������� ��������

		Protobuf::Bus* proto_bus = proto_catalog.add_buses();     //������� ����� ������ �������� � ��������� ���

		proto_bus->set_name(bus.name);
		proto_bus->set_circle(bus.circle);
		proto_bus->set_bus_wait_time(bus.bus_wait_time);
		proto_bus->set_bus_velocity(bus.bus_velocity);
		
		for (const auto& stop : catalog.GetRout(bus.name)) {      //��������� ������� ����� ������� id ���������
			proto_bus->add_route(stop->stop_id);
		}

	}

	for (const auto& [stops, distance] : catalog.GetDistances()) {          //�������� �� ���������� ��������
		
		Protobuf::Distance* proto_distance = proto_catalog.add_distances(); //������� ����� ������ ��������� � ��������� ���

		const auto& [stop_from, stop_to] = stops; 

		proto_distance->set_from_id(stop_from->stop_id);
		proto_distance->set_to_id(stop_to->stop_id);
		proto_distance->set_distance(distance);
	}


	return proto_catalog;                      
}

Protobuf::RenderSettings srlz::ConvertSettings(TransportCatalogue& catalog, renderer::RenderSettings& settings) {
	Protobuf::RenderSettings proto_settings;

	proto_settings.set_width(settings.size.x);
	proto_settings.set_height(settings.size.y);
	proto_settings.set_padding(settings.padding);
	proto_settings.set_stop_radius(settings.stop_radius);
	proto_settings.set_line_width(settings.line_width);
	proto_settings.set_bus_label_font_size(settings.bus_label_font_size);
	proto_settings.set_bus_label_offset_x(settings.bus_label_offset.x);
	proto_settings.set_bus_label_offset_y(settings.bus_label_offset.y);
	proto_settings.set_stop_label_font_size(settings.stop_label_font_size);
	proto_settings.set_stop_label_offset_x(settings.stop_label_offset.x);
	proto_settings.set_stop_label_offset_y(settings.stop_label_offset.y);
	proto_settings.set_underlayer_width(settings.underlayer_width);

	*(proto_settings.mutable_underlayer_color()) = ConvertColor(settings.underlayer_color);
	
	for (const auto& color : settings.color_palette) {
		*(proto_settings.add_color_palette()) = ConvertColor(color);
	}

	return proto_settings;
}

Protobuf::Color srlz::ConvertColor(svg::Color color) {
	Protobuf::Color proto_color;

	if (std::holds_alternative<svg::Rgb>(color)) {
		proto_color.set_index(1);
		proto_color.mutable_rgb()->set_red(std::get<svg::Rgb>(color).red);
		proto_color.mutable_rgb()->set_green(std::get<svg::Rgb>(color).green);
		proto_color.mutable_rgb()->set_blue(std::get<svg::Rgb>(color).blue);
	}
	else if (std::holds_alternative<svg::Rgba>(color)) {
		proto_color.set_index(2);
		proto_color.mutable_rgba()->set_red(std::get<svg::Rgba>(color).red);
		proto_color.mutable_rgba()->set_green(std::get<svg::Rgba>(color).green);
		proto_color.mutable_rgba()->set_blue(std::get<svg::Rgba>(color).blue);
		proto_color.mutable_rgba()->set_opacity(std::get<svg::Rgba>(color).opacity);
	}
	else if (std::holds_alternative<std::string>(color)) {
		proto_color.set_index(3);
		proto_color.set_text(std::get<std::string>(color));
	}
	else {
		proto_color.set_index(0);
	}

	return proto_color;
}

Protobuf::GraphEdges srlz::ConvertEdges(TransportRouter& transport_router) {
	Protobuf::GraphEdges proto_edges;

	for (const auto& edge : transport_router.GetEdges()) {
		*(proto_edges.add_edges()) = ConvertEdge(edge);
	}

	return proto_edges;
}

Protobuf::Edge srlz::ConvertEdge(const graph::Edge<double>& edge) {
	Protobuf::Edge proto_edge;

	proto_edge.set_from(edge.from);
	proto_edge.set_to(edge.to);
	proto_edge.set_weight(edge.weight);

	return proto_edge;
}

Protobuf::RouterSettings srlz::ConvertRouterSettings(TransportRouter& router) {
	Protobuf::RouterSettings proto_settings;

	proto_settings.set_bus_wait_tima(router.GetWait());
	proto_settings.set_bus_velocity(router.GetVelocity());

	return proto_settings;
}



//Deserialization
TransportCatalogue srlz::ConvertCatalogFromProto(Protobuf::TransportData& proto_catalog) {
	TransportCatalogue catalog;                                                    //������� ������������ �������

	for (const auto& proto_stop : *proto_catalog.mutable_stops()) {                //�������� �� ���������� ����� ������� � ��������� �� � �������
		catalog.AddStop(proto_stop.name(), proto_stop.x(), proto_stop.y());
	}


	for (const auto& proto_bus : *proto_catalog.mutable_buses()) {                 //�������� �� ��������� ����� ������� � ��������� �� � �������
		std::vector<std::string> route;

		for (const auto& id : proto_bus.route()) {                                 //�������� ������� ��������� �� id ���������� � ����� �������
			route.push_back(std::string(catalog.GetStopNameById(id)));
		}

		catalog.AddBus(proto_bus.name(), route, proto_bus.circle(), proto_bus.bus_wait_time(), proto_bus.bus_velocity());
			
	}

	for (const auto& proto_distance : *proto_catalog.mutable_distances()) {        //�������� �� ���������� ����� ������� � ��������� �� � �������
		catalog.AddDistance(std::string(catalog.GetStopNameById(proto_distance.from_id())), std::string(catalog.GetStopNameById(proto_distance.to_id())), proto_distance.distance());
	}

	return catalog;
}

renderer::RenderSettings srlz::ConvertSettingsFromProto(Protobuf::RenderSettings proto_settings) {
	renderer::RenderSettings settings;

	settings.size.x = proto_settings.width();
	settings.size.y = proto_settings.height();
	settings.padding = proto_settings.padding();
	settings.line_width = proto_settings.line_width();
	settings.stop_radius = proto_settings.stop_radius();
	settings.bus_label_font_size = proto_settings.bus_label_font_size();
	settings.bus_label_offset.x = proto_settings.bus_label_offset_x();
	settings.bus_label_offset.y = proto_settings.bus_label_offset_y();
	settings.stop_label_font_size = proto_settings.stop_label_font_size();
	settings.stop_label_offset.x = proto_settings.stop_label_offset_x();
	settings.stop_label_offset.y = proto_settings.stop_label_offset_y();
	settings.underlayer_width = proto_settings.underlayer_width();

	settings.underlayer_color = ConvertColorFromProto(proto_settings.underlayer_color());


	settings.color_palette.clear();
	for (const auto& proto_color : *proto_settings.mutable_color_palette()) {
		settings.color_palette.push_back(ConvertColorFromProto(proto_color));
	}

	return settings;
}

svg::Color srlz::ConvertColorFromProto(Protobuf::Color proto_color) {
	svg::Color color;

	if (proto_color.index() == 1) {
		svg::Rgb rgb;
		rgb.red = proto_color.rgb().red();
		rgb.green = proto_color.rgb().green();
		rgb.blue = proto_color.rgb().blue();
		color = rgb;
	}
	else if (proto_color.index() == 2) {
		svg::Rgba rgba;
		rgba.red = proto_color.rgba().red();
		rgba.green = proto_color.rgba().green();
		rgba.blue = proto_color.rgba().blue();
		rgba.opacity = proto_color.rgba().opacity();
		color = rgba;
	}
	else if (proto_color.index() == 3) {
		color = proto_color.text();
	}

	return color;
}

std::vector<graph::Edge<double>> srlz::ConvertEdgesFromProto(Protobuf::GraphEdges& proto_edges) {
	std::vector<graph::Edge<double>> edges;

	for (const auto& proto_edge : *proto_edges.mutable_edges()) {
		edges.push_back(ConvertEdgeFromProto(proto_edge));
	}

	return edges;
}

graph::Edge<double> srlz::ConvertEdgeFromProto(const Protobuf::Edge& proto_edge) {
	graph::Edge<double> edge;

	edge.from = proto_edge.from();
	edge.to = proto_edge.to();
	edge.weight = proto_edge.weight();

	return edge;
}
