#include "transport_catalogue.h"

void TransportCatalogue::AddStop(const std::string& name, double x, double y ) { //Добавление остановки
	if (stopname_to_stop_.count(name) == 0) {
		stops_.push_back({name,x,y,(stop_number_++)*2});
		stopname_to_stop_[stops_.back().name] = &stops_.back();
		stopname_to_buses_[stops_.back().name];
		stopid_to_stopname_[stops_.back().stop_id] = stops_.back().name;
	}
	else {
		stopname_to_stop_.at(name)->x = x;
		stopname_to_stop_.at(name)->y = y;
	}

}

std::string_view TransportCatalogue::GetStopNameById(size_t id) {
		return stopid_to_stopname_.at(id);
}

void TransportCatalogue::AddBus(const std::string& name, const std::vector <std::string>& route, bool circle, int bus_wait_time, double bus_velocity) { //Добавление маршрута 
	std::vector <Stop*> stops;
	stops.reserve(route.size());

	for (const std::string& stopname : route) {
		stops.push_back(stopname_to_stop_.at(stopname));
	}

	buses_.push_back({ name, circle, bus_wait_time, bus_velocity });
	busname_to_bus_[buses_.back().name] = &buses_.back();
	busname_to_route_[buses_.back().name] = stops;

	for (const auto& stop : busname_to_route_.at(buses_.back().name)) {
		stopname_to_buses_.at((*stop).name).insert(buses_.back().name);
	}
}


size_t TransportCatalogue::GetStopsCount() {
	return stops_.size();
}

Bus& TransportCatalogue::GetBus(const std::string& name) { //Получение маршрута
	return *busname_to_bus_.at(name);
}

std::vector<Stop*>& TransportCatalogue::GetRout(const std::string& name) {
	return busname_to_route_.at(name);
}

Stop& TransportCatalogue::GetStop(const std::string& name) { //Получение остановки
	return *stopname_to_stop_.at(name);
}

std::deque <Bus>& TransportCatalogue::GetBuses() { //Получение списка маршрутов
	return buses_;
}

std::deque <Stop>& TransportCatalogue::GetStops() { //Получение списка остановок
	return stops_;
}

bool TransportCatalogue::HasBusAtStop(const std::string& name) { //Проверка наличия маршрутов на остановке
	return stopname_to_buses_.at(name).size() != 0;
}

bool TransportCatalogue::HasBus(const std::string& name) { //Проверка наличия маршрута
	return busname_to_bus_.count(name)!=0;
}

bool TransportCatalogue::HasStop(const std::string& name) { //Проверка наличия остановки
	return stopname_to_stop_.count(name) != 0;
}

bool TransportCatalogue::IsCircle(const std::string& name) { //Проверка является ли маршрут кольцевым
	return GetBus(name).circle;
}
std::set<std::string_view>& TransportCatalogue::GetBuses(const std::string& name) { //Получение списка маршрутов остановки
	return stopname_to_buses_.at(name);
}

void TransportCatalogue::AddDistance(const std::string& stop1, const std::string& stop2, double distance) { //Добавление дистанций между остановками
	
	if (stopname_to_stop_.count(stop2) == 0) {
		std::string stop2copy = stop2;
		AddStop(stop2, 0.0, 0.0);
		real_distance_[{stopname_to_stop_.at(stop1), stopname_to_stop_.at(stop2copy)}] = distance;
	}
	else {
		real_distance_[{stopname_to_stop_.at(stop1), stopname_to_stop_.at(stop2)}] = distance;
	}
}

double TransportCatalogue::GetDistance(Stop* stop1, Stop* stop2) { //Получение дистанции между остановками
	if(real_distance_.count({ stop1,stop2 })!=0){
		return real_distance_.at({ stop1,stop2 });
	}
	else if (real_distance_.count({ stop2,stop1 }) != 0) {
		return real_distance_.at({ stop2,stop1 });
	}
	
	return 0;
}

double TransportCatalogue::GetDistanceEdge(Stop* stop1, Stop* stop2) { //Получение дистанции между остановками
	if (real_distance_.count({ stop1,stop2 }) != 0) {
		return real_distance_.at({ stop1,stop2 });
	}
	else{
		return 0;
	}
}

std::unordered_map<std::pair<Stop*, Stop*>, double, TransportCatalogue::StopsPairHasher> TransportCatalogue::GetDistances() {
	return real_distance_;
}