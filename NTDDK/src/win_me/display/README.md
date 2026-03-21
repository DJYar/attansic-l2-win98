# i910GML Test miniVDD (VxD) + Win32 ping tool

Минимальный диагностический miniVDD/VxD для Win98SE (без встраивания в display stack) и user-mode утилита `vxd_ping.exe`.

## Что реализовано

- Отдельный VxD `I910TEST.VXD` с `W32_DeviceIoControl` dispatch.
- Ранняя инициализация с попыткой открыть лог-файл:
  - `C:\I910TEST.LOG`
  - fallback: `C:\WINDOWS\I910TEST.LOG`
- Поиск PCI устройства Intel (`VEN_8086`) по списку DEV ID (в `pci.c`).
- Чтение из PCI config space: Vendor/Device/Revision/Command/BAR0/BAR2.
- Попытка map MMIO BAR0 через `_MapPhysToLinear`.
- IOCTL API:
  - `IOCTL_I910_GET_PCI_INFO`
  - `IOCTL_I910_READ_MMIO32`
  - `IOCTL_I910_WRITE_MMIO32`
  - `IOCTL_I910_MASK_INTERRUPTS` (stub, `ERROR_CALL_NOT_IMPLEMENTED`)
- Fail-safe:
  - при отсутствии PCI устройства драйвер не падает,
  - при отсутствии маппинга MMIO READ/WRITE возвращают ошибку,
  - offset alignment/size checks,
  - watchdog: после `I910_MMIO_WATCHDOG_LIMIT` ошибок MMIO доступ отключается.

## Структура

- `build/makefile` — сборка всего
- `minivdd/i910test/*` — VxD/miniVDD
- `user/vxd_ping/*` — Win32 console tool

## Сборка

1. Подготовить Win9x DDK окружение (как для `display/minivdd/3dlabsp2`).
2. Для VxD:
   - `cd i910_testvxd\minivdd\i910test`
   - `nmake`
3. Для user tool:
   - `cd i910_testvxd\user\vxd_ping`
   - `nmake`

Или из `i910_testvxd\build` выполнить `nmake`.

## Установка/загрузка

Вариант MVP (ручной):

1. Скопировать `I910TEST.VXD` в `C:\WINDOWS\SYSTEM` или `C:\WINDOWS\SYSTEM\IOSUBSYS`.
2. Добавить строку в `SYSTEM.INI` (секция `[386Enh]`):
   - `device=I910TEST.VXD`
3. Перезагрузить систему.

## Использование `vxd_ping.exe`

- Только получить PCI info:
  - `vxd_ping.exe`
- Прочитать MMIO DWORD:
  - `vxd_ping.exe 0`
  - `vxd_ping.exe 20`
- Записать MMIO DWORD (тестово):
  - `vxd_ping.exe 20 DEADBEEF`

## Ожидаемый лог

Лог в `C:\I910TEST.LOG` должен содержать этапы:

- `init: start`
- `pci: matched ...` или `pci: no supported intel device found`
- `mmio: mapped ...` или `mmio: map failed ...`
- `ioctl: ...`

## Замечания

- Это диагностический драйвер, не выполняет modeset/3D и не заменяет display `.DRV`.
- Размер BAR в MVP не вычисляется через probe-запись в PCI BAR; если неизвестен, используется fallback `16MB`.
