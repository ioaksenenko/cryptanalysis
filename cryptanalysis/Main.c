#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>


typedef struct {
	char* text; // �����
	int length; // ����� ������
} text_t;


typedef struct {
	char* word; // �����
	int length; // ����� �����
	int encoded_count; // ���������� ������������� ����
} word_t;


typedef struct {
	word_t* words; // ������ ����
	int count; // ���������� ����
} words_t;


typedef struct {
	words_t words; // ������ ����
	int length; // ����� �����
} group_t;


typedef struct {
	group_t* groups; // ������ �����
	int count; // ���������� �����
} groups_t;


typedef struct {
	char letter; // ����� ��������
	int frequency; // ������� ��� ����� ����������� � �����
	float probability; // ���������� ����������� ����� � ������ (frequency / ����� ���-�� ���� * 100%)
	char correspond; // ����� ������, ������� ������������� ����� letter
} letter_t;


text_t read(char* file_name) {
	FILE* f = fopen(file_name, "rb");
	if (f != NULL) {
		// �������� �������, ����� ����������� ���������� ��������� � ����� �����, ����� �������� ����� ������ � �����
		fseek(f, 0, SEEK_END);
		// ������� ftell ���������� �������� ��������������� ���������� ���� �� ������ ����� �� �������� ��������� ���������
		int file_size = ftell(f);
		// ���������� ��������� � ������ �����
		fseek(f, 0, SEEK_SET);
		text_t res = {
			.text = (char*)malloc(file_size * sizeof(char)),
			.length = file_size
		};
		for (int i = 0; i < file_size; i++) {
			res.text[i] = fgetc(f);
		}
		// � ����� ������� ���������� ��������� ����-����, ����� �� ��������� ������ �����
		res.text[res.length] = '\0';
		fclose(f);
		return res;
	}
	printf("������. ���� ������.");
	exit(EXIT_FAILURE);
}


void wtire(char* file_name, text_t text) {
	FILE* f = fopen(file_name, "wb");
	if (f != NULL) {
		fputs(text.text, f);
		fclose(f);
	} else {
		printf("�� ������� ������� ���� ��� ������.\n");
	}
}

// ������� ��� ��������� ������� ����� letter � ������� letters
// key �����, ����� ������ �� ����� ����� ����� �������� ������, ��������� � ������� letters � �������� ���� ���� letter - ����� �������� � ���� correspond - ��������������� �� ����� �������� ������
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


// ������� ��� ������������ ������� ����� � ������� ��������
// ���������� ������� tolower �������� ������� ��� �������� ��������
// ��� ����������� �������� �������� �� �����, ��� ������������� ����� ����������
char to_lower(char symbol) {
	return symbol <= -33 && symbol >= -64 ? symbol + 32 : symbol;
}


// ����������� ������� ��� ���������� � �������� ��������
char to_upper(char symbol) {
	return symbol <= -1 && symbol >= -32 ? symbol - 32 : symbol;
}


// ������� ��� �������� ����-�������� �� ������: ������ ����������, ����������� �������� � ��������, ������� ��� � ��������
// �� ���� ������� �� ���������, � ���������� �� ����������
// ���������� ������ � ��������� �������, ��������� ��� ����������� (���������� ������ �� �����) ����� ����� � ���������, �� ��� ������ ��������
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


// ������� ��� �������� ���������� ��������
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


// ������� ��� ��������� ������ � ������������ ��������� ������� � ������
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


// ���������� �������� �� �������� ������������
// ����� ��������
// ������� �������� ������� �������� letters
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


// ������� ��������� ���� ������, ��������������� �������� �� ������������
// ������� �������� ������� �������� letters
void set_corresponds(text_t text, letter_t* letters) {
	letter_t* sorted_letters = sort(letters);
	for (int i = 0; i < 32; i++) {
		letters[i].correspond = sorted_letters[i].letter;
	}
}


// ������� ��� ��������� ���������� ���� � ������
// ����� ��� �����������, ����� �� �������� ������ ������
int get_words_number(text_t text) {
	int res = 0;
	for (int i = 0; i < text.length; i++) {
		if (text.text[i] == ' ' && text.text[i - 1] != ' ') {
			res++;
		}
	}
	return res;
}


// ������� ����������� (���������� ������ �� ����� - ������)
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


// ������� ��� ������ ������ �� ����� ����� word_length ��� �� ���������� ��� ������������� ����
int search_group(groups_t groups, int word_length) {
	for (int i = 0; i < groups.count; i++) {
		if (groups.groups[i].length == word_length) {
			return i;
		}
	}
	return -1;
}


// ������� ����������� ���� �� �������� by
// ���������� by ��������� �������� "length" - ����������� �� ����� ����� ��� ����� ������ �������� - ����������� �� ���������� ��� ������������� ����
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


// ������� �������������
text_t decode(text_t text, letter_t* letters) {
	// ������� ����� �� ����-��������
	text_t without_stop_symbols = remove_stop_symbols(text, letters);
	// �� ��������
	text_t without_spaces = remove_spaces(without_stop_symbols);
	// ������������� ������� � �����������
	set_frequencies(without_spaces, letters);
	// ������������� ������������ ���� �������� � ������
	set_corresponds(without_spaces, letters);
	// ��������� ����� �� �����
	words_t words = tokenize(without_stop_symbols);
	// ���������� ����� �� �����
	groups_t word_length_groups = group_words(words, "length");

	// ���������� ��� �������� ������� �������������
	text_t* history = (text_t*)malloc(1024 * sizeof(text_t));
	// ��������� �� ������� ��������� ������������
	int histopy_pointer = 0;
	// ������� ��������� - �������� �����
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
		printf("�������� �����:\n1. �������� �������������� ������ � ������������ � ��������� ������������� ���� �������� ��������.\n2. �������� ��� �����, ��������������� �� ���������� ����.\n3. �������� ��� �����, ��������������� �� ���������� ���������������� �� ������ ������ ����.\n4. �������� ������������ � ��������� ��������������� �� ������ ������ ������.\n5. �������� ����� � ������������.\n6. ������������ ����� � ������������ � ���������.\n7. ������������ ���� �����.\n8. �������� ��������� �� ���������� ������.\n9. �������� �����.\n0. �����.\n\n");
		option = getch() - '0';
		if (option < 0 || option > 9) {
			printf("������������ ����� �����.\n");
		}
		if (option == 1) {
			printf("������ ������: ����� ������ -> ����� ��������\n");
			for (int i = 0; i < 32; i++) {
				printf("%c -> %c\n", letters[i].correspond, letters[i].letter);
			}
		}
		if (option == 2) {
			for (int i = 0; i < word_length_groups.count; i++) {
				printf("������ � ������ ����� %d: \n", word_length_groups.groups[i].length);
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
				printf("������ � ����������� ���������������� ���� %d: \n", groups.groups[i].length);
				for (int j = 0; j < groups.groups[i].words.count; j++) {
					printf("%s\n", groups.groups[i].words.words[j].word);
				}
			}
		}
		if (option == 4) {
			puts(history[histopy_pointer].text);
		}
		if (option == 5) {
			printf("������� �����, ������� ���������� ��������: ");
			scanf("\n%c", &input_letter);
			printf("������� �����, �� ������� ���������� ��������: ");
			scanf("\n%c", &correspond_letter);
		}
		if (option == 6) {
			printf("������� �����, ������� ���������� ������������: ");
			scanf("\n%c", &input_letter);
		}
		if (option == 8) {
			if (histopy_pointer > 0) {
				histopy_pointer--;
				printf("����� �� ���������� ������ ������� ��������!\n");
			} else {
				printf("����� ����������! ������������ ��������� � ��������� ���������.\n");
			}
		}
		if (option == 9) {
			printf("������� �����, ������� ���������� ��������: ");
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
					printf("\n����� ������� ��������!\n");
				}
				if (option == 6) {
					printf("\n����� ������� ������������!\n");
				}
				if (option == 7) {
					printf("����� ������� �����������!\n");
				}
				if (option == 9) {
					printf("\n����� ����� ������� ��������!\n");
				}
				histopy_pointer++;
			} else {
				if (option == 5 || option == 6) {
					printf("\n��������� ����� ��� ������������ ��� ��������!\n");
				}
				if (option == 7) {
					printf("����� ��� �����������!\n");
				}
				if (option == 9) {
					printf("\n��������� ����� ���� ������������, ���� �����������!\n");
				}
			}
		}
		printf("\n");
	} while (option != 0);

	return history[histopy_pointer];
}


int main() {
	// ������� ��� ����� ������� ��������
	SetConsoleCP(1251);
	// ������� ��� ������ ������� ��������
	SetConsoleOutputCP(1251);

	// ������� (����� ������������� � ������� �������� ������)
	letter_t letters[32] = { {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		}, {
			.letter = '�',
			.frequency = 0,
			.probability = 0,
			.correspond = NULL
		} };

	// ������ ������� ����
	text_t text = read("input.txt");
	// ���������� �����
	text_t decoded_text = decode(text, letters);
	// ���������� � �������� ���� ���������
	wtire("output.txt", decoded_text);

	// ��� ����� ������� �� ����������� ��� �������
	system("pause");
}