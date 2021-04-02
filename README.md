# AlphaSE_BasicUnit

  Library for writing firmware of modules of system extension module AlphaSE. The included example does not have any functionality, but can already be polled by the controller using the ADNet + and ModBus RTU protocols. You can use this sketch as a basis for developing new modules.
  
  Библиотека для написания прошивок модулей модуля расширения системы AlphaSE.
  Входящий в комплект пример не имеет никакого функционала, но уже может опрашиваться контроллером по протоколам
  ADNet+ и ModBus RTU. Данный скетч можно использовать в качестве основы для разработки новых модулей.

  Описание PIN:
  0 - RX RS485 RO,
  1 - TX RS485 DI,
  2 - управление приёмом и передачей по RS485. На этой же ноге висит Led индикации передачи,
  3 - Pin 1 для задания адреса модуля по шине ADNet/ModBus,
  4 - Pin 2 для задания адреса модуля по шине ADNet/ModBus,
  5 - Pin 3 для задания адреса модуля по шине ADNet/ModBus,
  6 - Pin 4 для задания адреса модуля по шине ADNet/ModBus,
  7 - Pin 5 для задания адреса модуля по шине ADNet/ModBus,
  8 - Pin 6 для задания адреса модуля по шине ADNet/ModBus,
  Адрес модуля вычисляется по формуле Pin1+Pin2*2+Pin3*4+Pin4*8+Pin5*16+Pin6*32,
  13 - Led индикации работы. Мигает 1 раз в секунду, если всё хорошо, иначе - ошибка

  Подробнее о системе AlphaSE можно узнать на сайте smart-elec.ru
  Author: Viktor S. Bykov
  Date: 02.10.2018г.
