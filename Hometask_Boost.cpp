#include "pch.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

std::vector<std::string> Processor(std::string req) //обработка введенной строки
{
	std::vector <std::string> string; //вводимая строка
	std::string sign; //знак
	auto s_pos = req.find("-"); //автоматическая продолжительность переменной, поиск в заданной строке знака "-"
	size_t pos = 0; //беззнаковое целое, нынешняя позиция 
	size_t n_pos; //беззнаковое целое, след. позиция

	if (s_pos != std::string::npos && req[s_pos] == '-') //проверка существования самого знака, его опеределение 
		sign = "-";
	else
		sign = "+";

	string.push_back(sign); //добавление знака в конец 
	while ((n_pos = req.find(sign, pos)) != std::string::npos) //пока строка существует
	{
		string.push_back(req.substr(pos, n_pos - pos)); //возвращает подстроку данной строки начиная с символа с индексом pos количеством (n_pos - pos)
		pos = n_pos + 1; //переход на след. позицию строки
	}
	string.push_back(req.substr(pos)); //добавление символа в конец вектора

	return string;
}

// http запрос методом GET и вывод ответа на экран
int main(int argc, char** argv) {    
	setlocale(0, "");

	std::string request;
	std::cin >> request; //ввод запроса

	std::vector <std::string> h_request = Processor(request); 

	try {
		//http://157.230.27.215/calc/diff/60/18 >> 60 - 18 = 42
		//http://157.230.27.215/calc/sum/73/26 >> 73 + 26 = 99
		int a = std::stoi(h_request[1]); //преобразование строки в целое число. Пропускаются все пробельные символы, до тех пор пока не встретится первый символ другого типа, затем берётся как можно больше символов, которые формируют корректную запись целочисленного числа и конвертирует её в значение нужного типа.
		int b = std::stoi(h_request[2]);
		std::string const host = "157.230.27.215";
		std::string const port = "80";
		std::stringstream url;
		std::string action;

		if (h_request[0] == "-") //выбор нужного обращения
			action = "/calc/diff/";
		else
			action = "/calc/sum/";

		url << action << a << "/" << b;

		std::string const target = url.str();
		int const version = 11;
		std::cout << target;
		
		net::io_context ioc;  //требуется для всех вводов-выводов

		// объекты исполняющие ф-цию ввода-вывода 
		tcp::resolver resolver(ioc); //распознаватель 
		auto const results = resolver.resolve(host, port); //шаблон локатора предоставляемых услуг 

		beast::tcp_stream stream(ioc); //используеся протокол TCP
		stream.connect(results); //функция connect используется процессом-клиентом для установления связи с сервером

		// настройка сообщения http запроса методом GET
		http::request<http::string_body> req{http::verb::get, target, version}; 
		//заголовки
		req.set(http::field::host, host);
		req.set(http::field::user_agent, "gg/test");

		http::write(stream, req); //отправка http запроса на удаленный хост

		beast::flat_buffer buffer; //буфер для чтения ОБЯЗАТЕЛЕН к сохранению

		http::response<http::dynamic_body> res; //контейнер для хранения ответа

		http::read(stream, buffer, res); //получение http ответа

		std::cout << res << std::endl; //вывод ответа

		beast::error_code ec; // код ошибки
		stream.socket().shutdown(tcp::socket::shutdown_both, ec); //закрытие сокета

		if (ec && ec != beast::errc::not_connected) //если произошла ошибка кроме not_connected 
			throw beast::system_error{ ec }; //сигнал о возникновении ошибки

	}
	catch (std::exception const& e) { //обработка исключений
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}