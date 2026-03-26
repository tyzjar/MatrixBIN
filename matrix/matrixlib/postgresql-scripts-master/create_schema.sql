DROP TABLE IF EXISTS car CASCADE;
DROP TABLE IF EXISTS material CASCADE;
DROP TABLE IF EXISTS counteragent CASCADE;
DROP TABLE IF EXISTS photo_number CASCADE;
DROP TABLE IF EXISTS vehicle CASCADE;
DROP TABLE IF EXISTS driver CASCADE;
DROP TABLE IF EXISTS photo_material CASCADE;
DROP TABLE IF EXISTS calibration CASCADE;
DROP TABLE IF EXISTS event CASCADE;
DROP TABLE IF EXISTS operator CASCADE;
DROP TABLE IF EXISTS device CASCADE;
DROP TABLE IF EXISTS train CASCADE;
DROP TABLE IF EXISTS scale CASCADE;

CREATE TABLE device (
  id bigserial NOT NULL PRIMARY KEY,
  name text NOT NULL,
  serial_number text NOT NULL
);

COMMENT ON COLUMN device.name IS 'Название устройства';
COMMENT ON COLUMN device.serial_number IS 'Сериальный номер';

CREATE TABLE train (
  id bigserial NOT NULL PRIMARY KEY,
  counteragent_id bigint,
  driver_id bigint,
  operator_id bigint,
  object_type smallint NOT NULL,
  sequential_number integer NOT NULL,
  motion_static_mode varchar(1) NOT NULL,
  auto_semiauto_mode varchar(1),
  direction varchar(2) NOT NULL,
  start_wgt_timestamp timestamp NOT NULL,
  finish_wgt_timestamp timestamp NOT NULL,
  gross_tare varchar(1) NOT NULL,
  qty_of_car smallint NOT NULL,
  gross_weight integer NOT NULL,
  tare_weight integer NOT NULL,
  netto_weight integer NOT NULL,
  netto_corrected integer NOT NULL,
  error_code smallint NOT NULL,
  error_description text
);

COMMENT ON COLUMN train.counteragent_id IS 'Ссылка на контрагента';
COMMENT ON COLUMN train.driver_id IS 'Ссылка на водителя';
COMMENT ON COLUMN train.operator_id IS 'Ссылка на оператора';
COMMENT ON COLUMN train.sequential_number IS 'Номер состава, получаемый из контроллера';
COMMENT ON COLUMN train.motion_static_mode IS 'Режим взвешивания : M - в движении, S - в статике';
COMMENT ON COLUMN train.auto_semiauto_mode IS 'Подрежимы в режиме взвешивания в движении: A - автомат, S - полуавтомат';
COMMENT ON COLUMN train.direction IS 'Направление при взвешивании: LR, RL';
COMMENT ON COLUMN train.start_wgt_timestamp IS 'Метка времени начала взвешивания';
COMMENT ON COLUMN train.finish_wgt_timestamp IS 'Метка времени окончания взвешивания';
COMMENT ON COLUMN train.gross_tare IS 'Тип взвешивания: T - тара, B - брутто';
COMMENT ON COLUMN train.qty_of_car IS 'Кол-во взвешенных транспортных средств';
COMMENT ON COLUMN train.error_code IS 'Результат взвешивания: 0 - взвесился, ? - номер ошибки';

CREATE TABLE material (
  id bigserial NOT NULL PRIMARY KEY,
  name text NOT NULL
);

COMMENT ON COLUMN material.name IS 'Название материала';

CREATE TABLE counteragent (
  id bigserial NOT NULL PRIMARY KEY,
  name text NOT NULL,
  INN text NOT NULL,
  KPP text NOT NULL,
  business_address text NOT NULL,
  mail_address text NOT NULL,
  director text NOT NULL,
  payment_account text NOT NULL,
  OGRN text NOT NULL,
  telephone text NOT NULL,
  email text NOT NULL,
  correspondent_account text NOT NULL,
  BIC text NOT NULL,
  bank_name text NOT NULL
);

COMMENT ON COLUMN counteragent.name IS 'Наименование предприятия';
COMMENT ON COLUMN counteragent.INN IS 'ИНН';
COMMENT ON COLUMN counteragent.business_address IS 'Юридический адрес';
COMMENT ON COLUMN counteragent.mail_address IS 'Почтовый адрес';
COMMENT ON COLUMN counteragent.director IS 'Директор';
COMMENT ON COLUMN counteragent.payment_account IS 'Расчетный счет';
COMMENT ON COLUMN counteragent.OGRN IS 'ОГРН';
COMMENT ON COLUMN counteragent.telephone IS 'Телефон';
COMMENT ON COLUMN counteragent.email IS 'Электронная почта';
COMMENT ON COLUMN counteragent.correspondent_account IS 'Корреспондентский счет';
COMMENT ON COLUMN counteragent.BIC IS 'БИК';

CREATE TABLE photo_number (
  id bigserial NOT NULL PRIMARY KEY,
  car_id bigint NOT NULL,
  photo bytea NOT NULL,
  zip boolean NOT NULL
);

COMMENT ON COLUMN photo_number.photo IS 'Фотография с номером транспортного средства';
COMMENT ON COLUMN photo_number.zip IS 'Сжатая фотография: 0 - нет, 1 - да';

CREATE TABLE car (
  id bigserial NOT NULL PRIMARY KEY,
  train_id bigint NOT NULL,
  material_id bigint,
  vehicle_id bigint,
  sequence_number smallint NOT NULL,
  car_type varchar(1) NOT NULL,
  gross_weight integer NOT NULL,
  tare_weight integer NOT NULL,
  netto_weight integer NOT NULL,
  netto_corrected integer,
  ratio real,
  start_wgt_timestamp timestamp,
  finish_wgt_timestampng timestamp,
  speed real,
  car_reg_number text,
  trailer_reg_number text,
  type_recog_numbe varchar(1),
  type_recog_trailer_number varchar(1),
  error_code smallint NOT NULL,
  error_description text,
  CONSTRAINT train_seq_number UNIQUE(train_id,sequence_number)	
);

COMMENT ON COLUMN car.sequence_number IS 'Порядковый номер';
COMMENT ON COLUMN car.car_type IS 'Тип транспортного средства';
COMMENT ON COLUMN car.gross_weight IS 'Вес брутто';
COMMENT ON COLUMN car.tare_weight IS 'Вес тары';
COMMENT ON COLUMN car.netto_weight IS 'Вес нетто';
COMMENT ON COLUMN car.netto_corrected IS 'Нетто с учетом ratio';
COMMENT ON COLUMN car.ratio IS 'Коэффициент пересчета нетто';
COMMENT ON COLUMN car.start_wgt_timestamp IS 'Метка времени начала взвешивания (только в статике)';
COMMENT ON COLUMN car.finish_wgt_timestampng IS 'Метка времени конца взвешивания (только в статике)';
COMMENT ON COLUMN car.speed IS 'Скорость при взвешивании в движении';
COMMENT ON COLUMN car.car_reg_number IS 'Номер транспортного средства';
COMMENT ON COLUMN car.type_recog_numbe IS 'Тип распознавания: A - автоматичски (система распознавания), M - в ручную (вводится оператором)';
COMMENT ON COLUMN car.error_code IS 'Результат взвешивания: 0 -  взвесился, ? - номер ошибки';
COMMENT ON COLUMN car.error_description IS 'Описание ошибки';

CREATE TABLE vehicle (
  id bigserial NOT NULL PRIMARY KEY,
  car_type varchar(1) NOT NULL,
  car_rec_number text NOT NULL,
  tare_weight integer NOT NULL
);

COMMENT ON COLUMN vehicle.car_type IS 'Тип транспортного средства';
COMMENT ON COLUMN vehicle.car_rec_number IS 'Номер транспортного средства';
COMMENT ON COLUMN vehicle.tare_weight IS 'Масса пустого транспортного средства';

CREATE TABLE driver (
  id bigserial NOT NULL PRIMARY KEY,
  surname text NOT NULL,
  name text NOT NULL,
  middlename text NOT NULL,
  fullname text NOT NULL,
  passport text
);

COMMENT ON COLUMN driver.surname IS 'Фамилия';
COMMENT ON COLUMN driver.name IS 'Имя';
COMMENT ON COLUMN driver.middlename IS 'Отчество';

CREATE TABLE photo_material (
  id bigserial NOT NULL PRIMARY KEY,
  car_id bigint NOT NULL,
  photo bytea NOT NULL,
  zip boolean NOT NULL
);

COMMENT ON COLUMN photo_material.photo IS 'Фотография с загруженным матералом';
COMMENT ON COLUMN photo_material.zip IS 'Сжатая фотография: 0 - нет, 1 - да';

CREATE TABLE calibration (
  id bigserial NOT NULL PRIMARY KEY,
  device_id bigint NOT NULL,
  timedate timestamp NOT NULL,
  description text NOT NULL
);

COMMENT ON COLUMN calibration.timedate IS 'Дата-время калибровки';
COMMENT ON COLUMN calibration.description IS 'Описание действий и установленных параметров';

CREATE TABLE event (
  id bigserial NOT NULL PRIMARY KEY,
  event_type smallint NOT NULL,
  event_src text NOT NULL,
  event_description text NOT NULL,
  timestamp timestamp NOT NULL
);

COMMENT ON COLUMN event.event_type IS 'Тип события';
COMMENT ON COLUMN event.event_src IS 'Источник события';
COMMENT ON COLUMN event.event_description IS 'Текст события';

CREATE TABLE operator (
  id bigint NOT NULL PRIMARY KEY,
  surname text NOT NULL,
  name text NOT NULL,
  middlename text NOT NULL,
  fullname text NOT NULL,
  password text NOT NULL,
  role varchar(500) NOT NULL
);

COMMENT ON COLUMN operator.surname IS 'Фамилия';
COMMENT ON COLUMN operator.name IS 'Имя';
COMMENT ON COLUMN operator.middlename IS 'Отчество';
COMMENT ON COLUMN operator.fullname IS 'ФИО';
COMMENT ON COLUMN operator.password IS 'Пароль';
COMMENT ON COLUMN operator.role IS 'Роль';

CREATE TABLE scale (
  description text NOT NULL
);

COMMENT ON COLUMN scale.description IS 'Описание весов';

ALTER TABLE calibration ADD CONSTRAINT calibration_id_device_fk FOREIGN KEY (device_id) REFERENCES device (id);
ALTER TABLE car ADD CONSTRAINT car_id_material_fk FOREIGN KEY (material_id) REFERENCES material (id);
ALTER TABLE car ADD CONSTRAINT car_id_train_fk FOREIGN KEY (train_id) REFERENCES train (id) ON DELETE CASCADE;
ALTER TABLE car ADD CONSTRAINT car_id_vehicle_fk FOREIGN KEY (vehicle_id) REFERENCES vehicle (id);
ALTER TABLE photo_material ADD CONSTRAINT photo_material_id_car_fk FOREIGN KEY (car_id) REFERENCES car (id) ON DELETE CASCADE;
ALTER TABLE photo_number ADD CONSTRAINT photo_number_id_car_fk FOREIGN KEY (car_id) REFERENCES car (id) ON DELETE CASCADE;
ALTER TABLE train ADD CONSTRAINT train_id_counteragent_fk FOREIGN KEY (counteragent_id) REFERENCES counteragent (id);
ALTER TABLE train ADD CONSTRAINT train_id_driver_fk FOREIGN KEY (driver_id) REFERENCES driver (id);
ALTER TABLE train ADD CONSTRAINT train_id_operator_fk FOREIGN KEY (operator_id) REFERENCES operator (id);
