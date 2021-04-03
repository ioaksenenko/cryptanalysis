#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>


typedef struct {
	char* text; // текст
	int length; // длина текста
} text_t;


typedef struct {
	char* word; // слово
	int length; // длина слова
	int encoded_count; // количество зашифрованных букв
} word_t;


typedef struct {
	word_t* words; // массив слов
	int count; // количество слов
} words_t;


typedef struct {
	words_t words; // массив слов
	int length; // длина слова
} group_t;


typedef struct {
	group_t* groups; // массив групп
	int count; // количество групп
} groups_t;


typedef struct {
	char letter; // буква алфавита
	int frequency; // сколько раз буква встречается в тесте
	float probability; // процентное соотношение буквы в тексте (frequency / общее кол-во букв * 100%)
	char correspond; // буква текста, которой соответствует буква letter
} letter_t;


text_t read(char* file_name) {
	FILE* f = fopen(file_name, "rb");
	if (f != NULL) {
		// вызываем функцию, чтобы переместить внутренний указатель в конец файла, чтобы получить длину текста в файле
		fseek(f, 0, SEEK_END);
		// функция ftell возвращает значение соответствующее количеству байт от начала файла до текущего положения указателя
		int file_size = ftell(f);
		// возвращаем указатель в начало файла
		fseek(f, 0, SEEK_SET);
		text_t res = {
			.text = (char*)malloc(file_size * sizeof(char)),
			.length = file_size
		};
		for (int i = 0; i < file_size; i++) {
			res.text[i] = fgetc(f);
		}
		// в конец массива необходимо поставить ноль-байт, чтобы не выводился лишний мусор
		res.text[res.length] = '\0';
		fclose(f);
		return res;
	}
	printf("Ошибка. Файл пустой.");
	exit(EXIT_FAILURE);
}


void wtire(char* file_name, text_t text) {
	FILE* f = fopen(file_name, "wb");
	if (f != NULL) {
		fputs(text.text, f);
		fclose(f);
	} else {
		printf("Не удалось открыть файл для записи.\n");
	}
}

// Функция для получения индекса буквы letter в массиве letters
// key нужен, чтобы понять по какой букве хотим получить индекс, поскольку в массиве letters у объектов есть поле letter - буква алфавита и поле correspond - соответствующая ей буква входного текста
int index(char letter, letter_t* letters, char* key) {
	for (int i = 0; i < 32; i++) {
		if (key == "letter" && letters[i].letter == letter) {
			return i;
		}
		if (key == "correspond" && letters[i].correspond == letter) {
			return i;
		}
	}
	return -1;
}


// функция для приведеления русской буквы к нижнему регистру
// встроенная функция tolower работает неверно для русского алфавита
// для английского алфавита работать не будет, при необходимости нужно доработать
char to_lower(char symbol) {
	return symbol <= -33 && symbol >= -64 ? symbol + 32 : symbol;
}


// аналогичная функция для приведения к верхнему регистру
char to_upper(char symbol) {
	return symbol <= -1 && symbol >= -32 ? symbol - 32 : symbol;
}


// Функция для удаления стоп-символов из текста: знаков пунктуации, управляющих символов и символов, которых нет в алфавите
// По сути символы не удаляются, а заменяются на пробельные
// Пробельные удалим в отдельной функции, поскольку для токенизации (разделению текста на слова) нужен текст с пробелами, но без лишних символов
text_t remove_stop_symbols(text_t text, letter_t* letters) {
	text_t res = {
		.text = (char*)malloc(text.length * sizeof(char)),
		.length = 0
	};
	for (int i = 0; i < text.length; i++) {
		char symbol = to_upper(text.text[i]);
		res.text[res.length] = index(symbol, letters, "letter") != -1 || symbol == ' ' ? text.text[i] : ' ';
		res.length++;
	}
	res.text[res.length] = '\0';
	return res;
}


// Функция для удаления пробельных символов
text_t remove_spaces(text_t text) {
	text_t res = {
		.text = (char*)malloc(text.length * sizeof(char)),
		.length = 0
	};
	for (int i = 0; i < text.length; i++) {
		if (text.text[i] != ' ') {
			res.text[res.length] = text.text[i];
			res.length++;
		}
	}
	res.text[res.length] = '\0';
	return res;
}


// Функция для установки частот и вероятностей появления символа в тексте
void set_frequencies(text_t text, letter_t* letters) {
	for (int i = 0; i < 32; i++) {
		for (int j = 0; j < text.length; j++) {
			if (letters[i].letter == text.text[j]) {
				letters[i].frequency++;
			}
		}
		letters[i].probability = (float)letters[i].frequency / text.length * 100;
	}
}


// Сортировка символов по убыванию вероятностей
// Метод пузырька
// Функция изменяет входной аргумент letters
letter_t* sort(letter_t* letters) {
	letter_t* res = (letter_t*)malloc(32 * sizeof(letter_t));
	for (int i = 0; i < 32; i++) {
		res[i] = (letter_t){
			.letter = letters[i].letter,
			.probability = letters[i].probability
		};
	}
	for (int i = 0; i < 31; i++) {
		for (int j = i + 1; j < 32; j++) {
			letter_t lhs = res[i];
			letter_t rhs = res[j];
			if (lhs.probability < rhs.probability) {
				res[i] = rhs;
				res[j] = lhs;
			}
		}
	}
	return res;
}


// Функция установки букв текста, соответствующих алфавиту по вероятностям
// Функция изменяет входной аргумент letters
void set_corresponds(text_t text, letter_t* letters) {
	letter_t* sorted_letters = sort(letters);
	for (int i = 0; i < 32; i++) {
		letters[i].correspond = sorted_letters[i].letter;
	}
}


// Функция для получения количества слов в тексте
// Нужна для токенизации, чтобы не выделять лишнюю память
int get_words_number(text_t text) {
	int res = 0;
	for (int i = 0; i < text.length; i++) {
		if (text.text[i] == ' ' && text.text[i - 1] != ' ') {
			res++;
		}
	}
	return res;
}


// Функция токенизации (разделения текста на слова - токены)
words_t tokenize(text_t text) {
	int words_number = get_words_number(text);
	words_t res = {
		.words = (word_t*)malloc(words_number * sizeof(word_t)),
		.count = words_number
	};
	for (int i = 0; i < res.count; i++) {
		res.words[i] = (word_t){
			.word = (char*)malloc(text.length * sizeof(char)),
			.length = 0,
			.encoded_count = 0
		};
	}
	for (int i = 0, j = 0; i < text.length; i++) {
		if (text.text[i] != ' ') {
			res.words[j].word[res.words[j].length] = text.text[i];
			res.words[j].length++;
			if (text.text[i] == (char)to_upper(text.text[i])) {
				res.words[j].encoded_count++;
			}
		}
		else {
			if (text.text[i - 1] != ' ') {
				res.words[j].word[res.words[j].length] = '\0';
				j++;
			}
		}
	}
	return res;
}


// Функция для поиска группы по длине слова word_length или по количеству ещё зашифрованных букв
int search_group(groups_t groups, int word_length) {
	for (int i = 0; i < groups.count; i++) {
		if (groups.groups[i].length == word_length) {
			return i;
		}
	}
	return -1;
}


// Функция группировки слов по признаку by
// Переменная by принимает значение "length" - группировка по длине слова или любое другое значение - группировка по количеству ещё зашифрованных букв
groups_t group_words(words_t words, char* by) {
	groups_t res = {
		.groups = (group_t*)malloc(words.count * sizeof(group_t)),
		.count = 0
	};
	for (int i = 0; i < words.count; i++) {
		int group_index = search_group(res, by == "length" ? words.words[i].length : words.words[i].encoded_count);
		if (group_index != -1) {
			res.groups[group_index].words.words[res.groups[group_index].words.count].word = words.words[i].word;
			res.groups[group_index].words.words[res.groups[group_index].words.count].length = words.words[i].length;
			res.groups[group_index].words.words[res.groups[group_index].words.count].encoded_count = words.words[i].encoded_count;
			res.groups[group_index].words.count++;
		}
		else {
			res.groups[res.count] = (group_t){
				.words = (words_t){
					.words = (word_t*)malloc(words.count * sizeof(word_t)),
					.count = 0
				},
				.length = by == "length" ? words.words[i].length : words.words[i].encoded_count
			};
			res.groups[res.count].words.words[res.groups[res.count].words.count].word = words.words[i].word;
			res.groups[res.count].words.words[res.groups[res.count].words.count].length = words.words[i].length;
			res.groups[res.count].words.words[res.groups[res.count].words.count].encoded_count = words.words[i].encoded_count;
			res.groups[res.count].words.count++;
			res.groups[res.count].length = by == "length" ? words.words[i].length : words.words[i].encoded_count;
			res.count++;
		}
	}
	return res;
}


// Функция декодирования
text_t decode(text_t text, letter_t* letters) {
	// очищаем текст от стоп-символов
	text_t without_stop_symbols = remove_stop_symbols(text, letters);
	// от пробелов
	text_t without_spaces = remove_spaces(without_stop_symbols);
	// устанавливаем частоты и вероятности
	set_frequencies(without_spaces, letters);
	// устанавливаем соответствие букв алфавита и текста
	set_corresponds(without_spaces, letters);
	// разбиваем текст на слова
	words_t words = tokenize(without_stop_symbols);
	// группируем слова по длине
	groups_t word_length_groups = group_words(words, "length");

	// переменная для хранения истории декодирования
	text_t* history = (text_t*)malloc(1024 * sizeof(text_t));
	// указатель на текущее состояние криптограммы
	int histopy_pointer = 0;
	// нулевое состояние - исходный текст
	history[histopy_pointer] = text;
	for (int i = 1; i < 1024; i++) {
		history[i] = (text_t){
			.text = (char*)malloc(text.length * sizeof(char)),
			.length = text.length
		};
		history[i].text[text.length] = '\0';
	}

	char input_letter;
	char correspond_letter;
	int option;
	do {
		printf("Выберите опцию:\n1. Показать предполагаемые замены в соответствии с частотами распределения букв русского алфавита.\n2. Показать все слова, сгруппированные по количеству букв.\n3. Показать все слова, сгруппированные по количеству нерасшифрованных на данный момент букв.\n4. Показать криптограмму с указанием расшифрованного на данный момент текста.\n5. Заменить букву в криптограмме.\n6. Декодировать букву в соответствии с частотами.\n7. Декодировать весь текст.\n8. Откатить изменения до предыдущей версии.\n9. Откатить букву.\n0. Выйти.\n\n");
		option = getch() - '0';
		if (option < 0 || option > 9) {
			printf("Некорректный номер опции.\n");
		}
		if (option == 1) {
			printf("Формат вывода: Буква текста -> Буква алфавита\n");
			for (int i = 0; i < 32; i++) {
				printf("%c -> %c\n", letters[i].correspond, letters[i].letter);
			}
		}
		if (option == 2) {
			for (int i = 0; i < word_length_groups.count; i++) {
				printf("Группа с длиной слова %d: \n", word_length_groups.groups[i].length);
				for (int j = 0; j < word_length_groups.groups[i].words.count; j++) {
					printf("%s\n", word_length_groups.groups[i].words.words[j].word);
				}
			}
		}
		if (option == 3) {
			text_t without_stop_symbols = remove_stop_symbols(history[histopy_pointer], letters);
			words_t words = tokenize(without_stop_symbols);
			groups_t groups = group_words(words, "encoded_count");
			for (int i = 0; i < groups.count; i++) {
				printf("Группа с количеством нерасшифрованных букв %d: \n", groups.groups[i].length);
				for (int j = 0; j < groups.groups[i].words.count; j++) {
					printf("%s\n", groups.groups[i].words.words[j].word);
				}
			}
		}
		if (option == 4) {
			puts(history[histopy_pointer].text);
		}
		if (option == 5) {
			printf("Введите букву, которую необходимо заменить: ");
			scanf("\n%c", &input_letter);
			printf("Введите букву, на которую необходимо заменить: ");
			scanf("\n%c", &correspond_letter);
		}
		if (option == 6) {
			printf("Введите букву, которую необходимо декодировать: ");
			scanf("\n%c", &input_letter);
		}
		if (option == 8) {
			if (histopy_pointer > 0) {
				histopy_pointer--;
				printf("Откат до предыдущей версии успешно выполнен!\n");
			} else {
				printf("Откат невозможен! Криптограмма находится в начальном состоянии.\n");
			}
		}
		if (option == 9) {
			printf("Введите букву, которую необходимо откатить: ");
			scanf("\n%c", &input_letter);
		}
		if (option == 5 || option == 6 || option == 7 || option == 9) {
			int is_decoded = 0;
			for (int i = 0; i < history[histopy_pointer].length; i++) {
				char upper_input_letter = to_upper(input_letter);
				char lower_input_letter = to_lower(input_letter);
				if ((option == 5 || option == 6) && history[histopy_pointer].text[i] == upper_input_letter || option == 7 || option == 9 && history[histopy_pointer].text[i] == lower_input_letter) {
					if (option == 5) {
						history[histopy_pointer + 1].text[i] = to_lower(correspond_letter);
						is_decoded = 1;
					}
					if (option == 6 || option == 7) {
						int idx = index(history[histopy_pointer].text[i], letters, "correspond");
						if (idx != -1) {
							history[histopy_pointer + 1].text[i] = to_lower(letters[idx].letter);
							is_decoded = 1;
						} else {
							history[histopy_pointer + 1].text[i] = history[histopy_pointer].text[i];
						}
					}
					if (option == 9) {
						history[histopy_pointer + 1].text[i] = to_upper(text.text[i]);
						is_decoded = 1;
					}
				} else {
					history[histopy_pointer + 1].text[i] = history[histopy_pointer].text[i];
				}
			}
			if (is_decoded) {
				if (option == 5) {
					printf("\nБуква успешно заменена!\n");
				}
				if (option == 6) {
					printf("\nБуква успешно декодирована!\n");
				}
				if (option == 7) {
					printf("Текст успешно декодирован!\n");
				}
				if (option == 9) {
					printf("\nОткат буквы успешно выполнен!\n");
				}
				histopy_pointer++;
			} else {
				if (option == 5 || option == 6) {
					printf("\nВведенная буква уже декодирована или заменена!\n");
				}
				if (option == 7) {
					printf("Текст уже декодирован!\n");
				}
				if (option == 9) {
					printf("\nВведенная буква либо закодирована, либо некорректна!\n");
				}
			}
		}
		printf("\n");
	} while (option != 0);

	return history[histopy_pointer];
}


int main() {
	// функция для ввода русских символов
	SetConsoleCP(1251);
	// функция для вывода русских символов
	SetConsoleOutputCP(1251);

	// алфавит (сразу упорядоченный в порядке убывания частот)
	letter_t letters[32] = { {
			.letter = 'О',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Е',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'А',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'И',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Н',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Т',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Р',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'С',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Л',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'В',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'К',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'П',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'М',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'У',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Д',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Я',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Ы',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Ь',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'З',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Б',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Г',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Й',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Ч',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Ю',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Х',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Ш',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Ж',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Щ',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Ц',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Ф',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Э',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = 'Ъ',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		} };

	// читаем входной файл
	text_t text = read("input.txt");
	// декодируем текст
	text_t decoded_text = decode(text, letters);
	// записываем в выходной файл результат
	wtire("output.txt", decoded_text);

	// это чтобы консоль не закрывалась при запуске
	system("pause");
}