import json

def find_indices():
    with open('qmk_firmware/keyboards/bastardkb/charybdis/4x6/info.json', 'r') as f:
        data = json.load(f)

    # Matrix coordinates for keys we want
    # 1-5, 6-0
    number_matrix = [
        [0, 1], [0, 2], [0, 3], [0, 4], [0, 5], # 1, 2, 3, 4, 5
        [5, 5], [5, 4], [5, 3], [5, 2], [5, 1]  # 6, 7, 8, 9, 0
    ]
    
    # Q-T, Y-P
    letter_matrix = [
        [1, 1], [1, 2], [1, 3], [1, 4], [1, 5], # Q, W, E, R, T
        [6, 5], [6, 4], [6, 3], [6, 2], [6, 1]  # Y, U, I, O, P
    ]

    rgb_layout = data['rgb_matrix']['layout']
    
    # Map matrix to index
    matrix_to_index = {}
    for idx, led in enumerate(rgb_layout):
        if 'matrix' in led:
            r, c = led['matrix']
            matrix_to_index[(r, c)] = idx

    print("Number Key Indices:")
    num_indices = []
    for r, c in number_matrix:
        idx = matrix_to_index.get((r, c), -1)
        num_indices.append(idx)
    print(", ".join(map(str, num_indices)))

    print("\nLetter Key Indices:")
    let_indices = []
    for r, c in letter_matrix:
        idx = matrix_to_index.get((r, c), -1)
        let_indices.append(idx)
    print(", ".join(map(str, let_indices)))

if __name__ == "__main__":
    find_indices()
