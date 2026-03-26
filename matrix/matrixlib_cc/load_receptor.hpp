#ifndef __LOAD_RECEPTOR_HPP__
#define __LOAD_RECEPTOR_HPP__  

/* ГПУ - грузоприемное устройство
 * на всю систему одно ГПУ, в которых описаны все ГП(платформы) для всех весов
 */

#include "platform.hpp"

class LoadReceptor {
	private:
		std::string			  description;		// описание ГПУ
		std::vector<Platform> platforms;
	public:
		LoadReceptor();
		~LoadReceptor();
// Функции добавления, редактирования, удаления платформ
	
};
#endif
