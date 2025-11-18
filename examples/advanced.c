/* Advanced C features demonstration */

int get_day_type(int day) {
    int type;

    switch (day) {
        case 1:
        case 7:
            type = 0;
            break;
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            type = 1;
            break;
        default:
            type = -1;
            break;
    }

    return type;
}

int calculate_bits(int value) {
    int count = 0;
    int mask = 1;
    int i;

    for (i = 0; i < 8; i++) {
        int bit = value & mask;
        if (bit) {
            count++;
        }
        mask = mask << 1;
    }

    return count;
}

int main() {
    int x = 10;
    int y = 5;
    int result;

    /* Ternary operator */
    result = (x > y) ? x : y;

    /* Compound assignments */
    x += 5;
    y *= 2;
    x -= 3;

    /* Bitwise operations */
    int flags = 0;
    flags |= 1;
    flags |= 4;
    int masked = flags & 5;
    int shifted = x << 2;
    int inverted = ~flags;

    /* Do-while loop */
    int counter = 0;
    do {
        counter++;
        x--;
    } while (x > 0);

    /* sizeof */
    int size = sizeof(int);

    return 0;
}
