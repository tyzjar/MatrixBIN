DROP TABLE IF EXISTS car;
DROP TABLE IF EXISTS train;

CREATE TABLE IF NOT EXISTS train (
	id SERIAL PRIMARY KEY,		-- id поезда (основной ключ)
	num INTEGER NOT NULL,	    -- номер поезда
	mode VARCHAR(1) NOT NULL,			-- в статике/в движении S/M
	mode_auto VARCHAR(1) NOT NULL,		-- в движении, автомат/полуавтомат A/S
	direct VARCHAR(2) NOT NULL,			-- в движении, направление движения LR/RL
	start_weighing	TIMESTAMP NOT NULL,	-- начало взвешивания
	finish_weighing TIMESTAMP NOT NULL,	-- окончание взвешивания
	type_weighing VARCHAR(1) NOT NULL,	-- тип взвешивания, тара/брутто T/B
	cnt_car	SMALLINT NOT NULL,			-- кол-во вагонов
	weight INTEGER NOT NULL,				-- общий вес состава
	result SMALLINT NOT NULL,			-- результат взвешивани: ок(0) или номер ошибки
	error TEXT					-- текст ошибки
);

CREATE TABLE IF NOT EXISTS car (
	train_id integer REFERENCES train(id)	-- id поезда (вторичный ключ)
		ON DELETE CASCADE,
	num	SMALLINT NOT NULL,					-- номер вагона
	car_type VARCHAR(1) NOT NULL,			-- тип вагон(число осей)  или локомотив L
	weight INTEGER NOT NULL,				-- вес вагона
	speed	REAL NOT NULL,					-- скорость при взвешивании в движении
	result	SMALLINT NOT NULL,				-- результат взвешивания
	error TEXT,								-- текст ошибки: ок(0) или номер ошибки
	CONSTRAINT car_id PRIMARY KEY(train_id,num)	-- основной ключ
);


