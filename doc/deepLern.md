2026-04-29 16:30 | GPT-5.4 Thinking-Mini | №18

В MSYS2/UCRT64 проще всего так:

# 1) Все include из cpp/hpp

```bash
grep -RhoP '^#include\s*[<"]\K[^>"]+' ./src ./include
```

---

# 2) Уникальные include

```bash
grep -RhoP '^#include\s*[<"]\K[^>"]+' ./src ./include | sort -u
```

---

# 3) Только системные `<...>`

```bash
grep -RhoP '^#include\s*<\K[^>]+' ./src ./include | sort -u
```

---

# 4) Только локальные `"..."`

```bash
grep -RhoP '^#include\s*"\K[^"]+' ./src ./include | sort -u
```

---

# 5) С указанием файлов-источников

```bash
grep -RHP '^#include\s*[<"].*[>"]' ./src ./include
```

---

# 6) Очень полезно: частота использования include

```bash
grep -RhoP '^#include\s*[<"]\K[^>"]+' ./src ./include \
| sort \
| uniq -c \
| sort -nr
```

Получишь:

```txt id="1h2hrg"
15 windows.h
12 string
10 filesystem
8 vector
...
```

Это уже хороший detector:

* тяжёлых заголовков,
* STL creep,
* accidental dependencies.

---

# 7) Если хочешь увидеть «кто тянет windows.h»

```bash
grep -RHP '#include\s*[<"]windows\.h[>"]' ./src ./include
```

---

# 8) Очень полезный audit для тебя

## STL usage

```bash
grep -RhoP '^#include\s*<\K[^>]+' ./src ./include \
| grep -E 'iostream|filesystem|regex|thread|future|locale|format|ranges'
```

---

# 9) Ещё полезнее — dependency graph

GCC умеет:

```bash
g++ -MM src/main.cpp
```

или:

```bash
g++ -M src/main.cpp
```

---

## Разница

### `-MM`

без системных include.

### `-M`

со всеми системными include.

---

# 10) Для всего проекта

Например:

```bash
for f in ./src/*.cpp; do
    echo "=== $f ==="
    g++ -MM "$f"
done
```

---

# 11) Самый полезный режим именно для тебя

Я бы сделал:

```bash
grep -RhoP '^#include\s*[<"]\K[^>"]+' ./src ./include \
| sort \
| uniq -c \
| sort -nr > ./doc/include_audit.txt
```

И периодически сравнивал.

Так очень быстро видно:

* когда в проект заползает тяжёлый STL,
* когда кто-то начал тащить shell helper APIs,
* когда появилась threading/runtime инфраструктура.
