#include "transport_router.h"

TransportRouter::TransportRouter(TransportCatalogue& catalog, int bt, double bv) :catalog_(catalog), bus_wait_time_(bt), bus_velocity_(bv) {
	graph_ = std::make_unique <graph::DirectedWeightedGraph<double>>(catalog.GetStopsCount() * 2);//������� ���� � ����������� ������ �� 2 �� ���������
	FillGraph();
	router_ = std::make_unique<graph::Router <double>>(*graph_);
}


void TransportRouter::FillGraph() {

	for (Stop& stop : catalog_.GetStops()) {//�������� �� ���� ����������
		graph::Edge<double> edge = { stop.stop_id, stop.stop_id + 1, bus_wait_time_ };

		edge_info_[graph_->AddEdge(edge)] = std::make_unique < EdgeInfo >(stop.name, 0, stop.stop_id); //��������� ����� � ���� � ���������� � ������� (name, span_count, stop_id)
	}

	for (Bus& bus : catalog_.GetBuses()) {//�������� �� ���� ���������
		if (bus.circle) {
			AddCircleBusEdges(bus);
		}
		else {
			AddLineBusEdges(bus);
		}
	}
}

void TransportRouter::FillGraphFromProto(std::vector<graph::Edge<double>>& edges) {
	for (const auto& edge : edges) {
		//edge_info_[graph_->AddEdge(edge)] = std::make_unique < EdgeInfo >(bus.name, std::distance(stop1, stop2), (*stop2)->stop_id);

	}
}

void TransportRouter::AddBusEdges(Bus& bus, std::vector<Stop* >::iterator begin, std::vector<Stop* >::iterator end) {
	for (auto stop1 = begin; stop1 != end - 1; ++stop1) {                      //�������� �� ���� ���������� ��������

		double velocity = bus.bus_velocity;
		double all_distance_time = 0;                                        //��������� �� ������� ��������� �� stop2 ��� ���������
		std::vector<Stop* >::iterator prev_stop = stop1;

		bool first_distance = true;
		for (auto stop2 = stop1 + 1; stop2 != end; ++stop2) {                //�������� �� ���������� ���� �� �������� ����� ���������� �����
			double distance = catalog_.GetDistanceEdge(*stop1, *stop2);

			if (!first_distance || distance == 0) {                                            //���� � �������� ���� ������ ���� �� stop1 � stop2
				distance = catalog_.GetDistance(*prev_stop, *stop2);

			}


			double ride_time = distance / velocity;
			all_distance_time += ride_time;
			graph::Edge<double> edge = { (*stop1)->stop_id + 1, (*stop2)->stop_id, all_distance_time };
			edge_info_[graph_->AddEdge(edge)] = std::make_unique < EdgeInfo >(bus.name, std::distance(stop1, stop2), (*stop2)->stop_id);
			prev_stop = stop2;
			first_distance = false;
		}
	}
}


void TransportRouter::AddCircleBusEdges(Bus& bus) {
	AddBusEdges(bus, catalog_.GetRout(bus.name).begin(), catalog_.GetRout(bus.name).end());
}


void TransportRouter::AddLineBusEdges(Bus& bus) {
	size_t mid_num = (catalog_.GetRout(bus.name).size() - 1) / 2;                        //������� ������ �������� �������� 
	std::vector<Stop* >::iterator mid = catalog_.GetRout(bus.name).begin() + mid_num;    //�������� �������� ��������

	AddBusEdges(bus, catalog_.GetRout(bus.name).begin(), mid + 1);
	AddBusEdges(bus, mid, catalog_.GetRout(bus.name).end());
}


void TransportRouter::ProcessRoute(json::Builder& builder, const json::Node& request, size_t from, size_t to) {
	std::optional<graph::Router<double>::RouteInfo> info = router_->BuildRoute(from, to);

	if (!info.has_value()) {//���� ��� �������� ������� ���������

		builder.StartDict();
		builder.Key("request_id").Value((request.AsDict()).at("id").AsInt());     // ���������� id ������� � ���� ������
		builder.Key("error_message").Value(std::string(std::string("not found")));
		builder.EndDict();
	}
	else {
		builder.StartDict();
		double time = info.value().weight;                                        //����� ����� 

		builder.Key("request_id").Value((request.AsDict()).at("id").AsInt());     // ���������� id ������� � ���� ������
		builder.Key("total_time").Value(time);                                    // ���������� ������� ������� ��������� ��� ����������� ��������



		builder.Key("items").StartArray();                                        // ���������� ������� items


		bool wait = true;

		for (auto edgeid : info.value().edges) {                                  //��������� �� ������� EdgeId
			auto& edge = edge_info_.at(edgeid);                                   //�������� Edge �� ��� id 
			if (!wait) {
				wait = true;

				builder.StartDict();
				builder.Key("type").Value(std::string("Bus"));
				builder.Key("bus").Value(edge->name);                             //��� �������� 
				builder.Key("span_count").Value(edge->stop_count);                //���������� ���������
				builder.Key("time").Value(graph_->GetEdge(edgeid).weight);        //����� �������� 
				builder.EndDict();
			}
			else {
				wait = false;
				std::string_view stopname = catalog_.GetStopNameById(edge_info_.at(edgeid)->stopid);

				builder.StartDict();
				builder.Key("type").Value(std::string("Wait"));
				builder.Key("stop_name").Value(std::string(stopname));
				builder.Key("time").Value(bus_wait_time_);
				builder.EndDict();
			}

		}

		builder.EndArray();//items

		builder.EndDict();//query
	}
}

std::vector<graph::Edge<double>>& TransportRouter::GetEdges() {
	return graph_->GetEdges();
}

int TransportRouter::GetWait() {
	return bus_wait_time_;
}
double TransportRouter::GetVelocity() {
	return bus_velocity_;
}

