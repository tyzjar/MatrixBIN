#ifndef __PLATFORM_HPP__
#define __PLATFORM_HPP__  

class Section {
	private:
		unsigned id;			// номер секции (1...N)
		std::string	description; // описание секции
		unsigned liveRail;		// взешиваемый участок
		unsigned deadRail;		// невзвешиваемый участок
	public:
		Section(unsigned);
		~Section();
};

class Platform {
	private:
		unsigned id;					// номер платформы (1...N)
		std::string		 	description;// описание платформы
		std::vector<Section> sections;	// секции платформы
	public:
		Platform(unsigned);
		~Platform();
// Функции добавления, редактирования, удаления секций
	
};

#endif
