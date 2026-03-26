#ifndef __PLATFORM_HPP__
#define __PLATFORM_HPP__  

class Section {
	private:
		unsigned id;
		unsigned liveRail;		// взешиваемый участок
		unsigned deadRail;		// невзвештваемый участок
	public:
		Section(unsigned);
		~Section();
};

class Platform {
	private:
		unsigned id;
		std::vector<Section> sections;	// секции платформы
	public:
		Platform(unsigned);
		~Platform();
};

#endif
