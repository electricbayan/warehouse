ФИО: Волкович Артём Дмитриевич
Тема: "Построение маршрута для складского погрузчика"
Описание: консольная программа, которая будет по заданным данным оптимально строить маршрут для складского погрузчика
- Создание собственной карты склада
- Добавление/удаление имеющихся на складе товаров в 3Д
- 

Список комманд:
make-warehouse <warehouse-name> <x1> <y1> <x2> <y2> . . .
inspect-warehouse <warehouse-name>
delete-warehouse <warehouse-name>

add-shelf <warehouse-name> <shelf-name> <shelf-floors> <x1> <y1> <x2> <y2>
delete-shelf <warehouse-name><shelf-name>
inspect-shelf <warehouse-name> <shelf-name>
update-shelf <warehouse-name> <shelf-name> <shelf-floors> <x1> <y1> <x2> <y2>

add-item <warehouse-name> <shelf-name> <floor> <item-name> <quantity>
delete-item <warehouse-name> <shelf-name> <item-name>
update-item <warehouse-name> <shelf-name> <item-name> <quantity>
inspect-item <warehouse-name> <shelf-name> <item-name>

make-route <warehouse-name> <item-name> <x> <y>

Описание команд:
make-warehouse создаёт помещение склада из n точек, всё что внутри точек считается зоной где можно размещать полки и находиться погрузчику
Команда некорректна если точек меньше трёх либо они образуют прямую
inspect-warehouse показывает список координат склада
delete-warehouse удаляет имеющийся склад

add-shelf создаёт полку для товаров, shelf-floors - количество этажей полки. для упрощения будем считать полку всегда прямоугольной, её координаты задаются координатами противоположных вершин прямоугольника. Все товары склада будут храниться в хэш таблице.
Команда некорректна если прямоугольник не вписывается в плоскость склада
delete-shelf, inspect-shelf по аналогии с inspect-warehouse и delete-warehouse
update-shelf меняет высоту полки либо её координаты

add-item создаёт товар и помещает его на имеющуюся полку в заданном количестве
delete-item, update-item, inspect-item по аналогии с предыдущими командами.

make-route создаёт маршрут от имеющейся точки к нужному товару. Будем считать что внутри склада всё незанятое полками пространство является пригодным для передвижения погрузчика. Тогда всю плоскость можно разбить на сетку и добавить туда имеющиеся полки. Сетка будет являться графом, расстояние между точками которого можно найти с помощью алгоритма Дейкстры. Полученный маршрут можно сгладить диагоналями.

 
