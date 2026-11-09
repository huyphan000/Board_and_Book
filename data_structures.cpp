#ifndef BOARD_AND_BOOK_DATA_STRUCTURES_CPP
#define BOARD_AND_BOOK_DATA_STRUCTURES_CPP

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include "class_book_and_boardgame_docfile.cpp"

using namespace std;

// ============================================================
// 1. LINKED LIST CHO STRING
// ============================================================

// Node lưu một chuỗi và địa chỉ của node tiếp theo.
struct NodeString {
    string data;
    NodeString* next;

    explicit NodeString(const string& val) : data(val), next(nullptr) {}
};

// Danh sách liên kết đơn lưu các chuỗi.
class LinkedListString {
private:
    NodeString* head;
    NodeString* tail;
    int size;

    void copyFrom(const LinkedListString& other) {
        try {
            for (NodeString* current = other.head;
                 current != nullptr;
                 current = current->next) {
                insertBack(current->data);
            }
        } catch (...) {
            clear();
            throw;
        }
    }

public:
    LinkedListString() : head(nullptr), tail(nullptr), size(0) {}

    // Sao chép sâu để hai danh sách không dùng chung vùng nhớ.
    LinkedListString(const LinkedListString& other)
        : head(nullptr), tail(nullptr), size(0) {
        copyFrom(other);
    }

    LinkedListString& operator=(const LinkedListString& other) {
        if (this != &other) {
            LinkedListString temp(other);
            swap(temp);
        }
        return *this;
    }

    // Chuyển quyền sở hữu các node, không sao chép từng node.
    LinkedListString(LinkedListString&& other) noexcept
        : head(other.head), tail(other.tail), size(other.size) {
        other.head = nullptr;
        other.tail = nullptr;
        other.size = 0;
    }

    LinkedListString& operator=(LinkedListString&& other) noexcept {
        if (this != &other) {
            clear();
            head = other.head;
            tail = other.tail;
            size = other.size;

            other.head = nullptr;
            other.tail = nullptr;
            other.size = 0;
        }
        return *this;
    }

    ~LinkedListString() {
        clear();
    }

    void swap(LinkedListString& other) noexcept {
        std::swap(head, other.head);
        std::swap(tail, other.tail);
        std::swap(size, other.size);
    }

    // Chèn chuỗi vào đầu danh sách.
    void insertFront(const string& val) {
        NodeString* newNode = new NodeString(val);
        newNode->next = head;
        head = newNode;

        if (tail == nullptr) {
            tail = newNode;
        }

        ++size;
    }

    // Chèn chuỗi vào cuối danh sách: O(1) nhờ có con trỏ tail.
    void insertBack(const string& val) {
        NodeString* newNode = new NodeString(val);

        if (head == nullptr) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }

        ++size;
    }

    // Lấy giá trị ở đầu danh sách nhưng không xóa.
    bool getFront(string& val) const {
        if (head == nullptr) {
            return false;
        }

        val = head->data;
        return true;
    }

    // Lấy và xóa node đầu tiên. Hàm này được QueueString sử dụng.
    bool removeFront(string& val) {
        if (head == nullptr) {
            return false;
        }

        NodeString* toDelete = head;
        val = toDelete->data;
        head = head->next;

        if (head == nullptr) {
            tail = nullptr;
        }

        delete toDelete;
        --size;
        return true;
    }

    // Xóa lần xuất hiện đầu tiên của chuỗi cần tìm.
    bool remove(const string& val) {
        if (head == nullptr) {
            return false;
        }

        if (head->data == val) {
            string ignored;
            return removeFront(ignored);
        }

        NodeString* previous = head;
        NodeString* current = head->next;

        while (current != nullptr && current->data != val) {
            previous = current;
            current = current->next;
        }

        if (current == nullptr) {
            return false;
        }

        previous->next = current->next;

        if (current == tail) {
            tail = previous;
        }

        delete current;
        --size;
        return true;
    }

    bool contains(const string& val) const {
        for (NodeString* current = head;
             current != nullptr;
             current = current->next) {
            if (current->data == val) {
                return true;
            }
        }
        return false;
    }

    int getSize() const {
        return size;
    }

    bool isEmpty() const {
        return head == nullptr;
    }

    // Xóa toàn bộ node, đồng thời đưa danh sách về trạng thái rỗng.
    void clear() {
        while (head != nullptr) {
            NodeString* toDelete = head;
            head = head->next;
            delete toDelete;
        }

        tail = nullptr;
        size = 0;
    }

    // Hiển thị danh sách. Có thể truyền nhãn khác khi dùng cho Queue.
    void display(const string& label = "LinkedList") const {
        if (!label.empty()) {
            cout << label << ": ";
        }

        NodeString* current = head;
        while (current != nullptr) {
            cout << current->data << " -> ";
            current = current->next;
        }

        cout << "NULL" << endl;
    }
};


// ============================================================
// 2. STACK CHO INT
// ============================================================

// Node lưu một số nguyên.
struct NodeInt {
    int data;
    NodeInt* next;

    explicit NodeInt(int val) : data(val), next(nullptr) {}
};

// Stack hoạt động theo nguyên tắc LIFO (vào sau, ra trước).
class StackInt {
private:
    NodeInt* top;
    int size;

    void copyFrom(const StackInt& other) {
        // Sao chép từ đỉnh xuống đáy và giữ nguyên thứ tự các phần tử.
        NodeInt* current = other.top;
        NodeInt* last = nullptr;

        try {
            while (current != nullptr) {
                NodeInt* newNode = new NodeInt(current->data);

                if (top == nullptr) {
                    top = newNode;
                } else {
                    last->next = newNode;
                }

                last = newNode;
                current = current->next;
                ++size;
            }
        } catch (...) {
            clear();
            throw;
        }
    }

public:
    StackInt() : top(nullptr), size(0) {}

    StackInt(const StackInt& other) : top(nullptr), size(0) {
        copyFrom(other);
    }

    StackInt& operator=(const StackInt& other) {
        if (this != &other) {
            StackInt temp(other);
            swap(temp);
        }
        return *this;
    }

    StackInt(StackInt&& other) noexcept
        : top(other.top), size(other.size) {
        other.top = nullptr;
        other.size = 0;
    }

    StackInt& operator=(StackInt&& other) noexcept {
        if (this != &other) {
            clear();
            top = other.top;
            size = other.size;

            other.top = nullptr;
            other.size = 0;
        }
        return *this;
    }

    ~StackInt() {
        clear();
    }

    void swap(StackInt& other) noexcept {
        std::swap(top, other.top);
        std::swap(size, other.size);
    }

    // Thêm phần tử lên đỉnh stack.
    void push(int val) {
        NodeInt* newNode = new NodeInt(val);
        newNode->next = top;
        top = newNode;
        ++size;
    }

    // Lấy phần tử trên đỉnh và xóa nó khỏi stack.
    // Trả về false nếu stack đang rỗng.
    bool pop(int& val) {
        if (top == nullptr) {
            return false;
        }

        NodeInt* toDelete = top;
        val = toDelete->data;
        top = top->next;

        delete toDelete;
        --size;
        return true;
    }

    // Đọc giá trị trên đỉnh nhưng không xóa.
    bool topValue(int& val) const {
        if (top == nullptr) {
            return false;
        }

        val = top->data;
        return true;
    }

    bool isEmpty() const {
        return top == nullptr;
    }

    int getSize() const {
        return size;
    }

    void clear() {
        while (top != nullptr) {
            NodeInt* toDelete = top;
            top = top->next;
            delete toDelete;
        }

        size = 0;
    }

    void display() const {
        cout << "Stack (top -> bottom): ";

        for (NodeInt* current = top;
             current != nullptr;
             current = current->next) {
            cout << current->data << " ";
        }

        cout << endl;
    }
};


// ============================================================
// 3. QUEUE CHO STRING
// ============================================================

// Queue hoạt động theo nguyên tắc FIFO (vào trước, ra trước).
// Queue dùng LinkedListString để lưu dữ liệu.
class QueueString {
private:
    LinkedListString list;

public:
    // Thêm phần tử vào cuối hàng đợi.
    void enqueue(const string& val) {
        list.insertBack(val);
    }

    // Lấy phần tử ở đầu hàng đợi và xóa phần tử đó.
    bool dequeue(string& val) {
        return list.removeFront(val);
    }

    // Xem phần tử đầu hàng đợi nhưng không xóa.
    bool frontValue(string& val) const {
        return list.getFront(val);
    }

    bool isEmpty() const {
        return list.isEmpty();
    }

    int getSize() const {
        return list.getSize();
    }

    void clear() {
        list.clear();
    }

    // Hiển thị đầy đủ các phần tử trong hàng đợi.
    void display() const {
        cout << "Queue (size: " << getSize() << ") - Front -> ";
        // Hàm display của danh sách sẽ in các phần tử và NULL.
        // Nhãn rỗng để tránh in lặp chữ Queue.
        list.display("");
    }
};


// ============================================================
// 4. MAP CHO STRING -> INT
// ============================================================

// Map đơn giản lưu cặp key-value bằng hai mảng song song.
// Giới hạn tối đa 100 cặp để phù hợp với ví dụ cấu trúc dữ liệu cơ bản.
class SimpleMapStringInt {
private:
    static const int CAPACITY = 100;

    string keys[CAPACITY];
    int values[CAPACITY];
    int size;

public:
    SimpleMapStringInt() : size(0) {}

    // Thêm key mới hoặc cập nhật value nếu key đã tồn tại.
    // Trả về false nếu map đã đầy và không thể thêm key mới.
    bool insert(const string& key, int value) {
        for (int i = 0; i < size; ++i) {
            if (keys[i] == key) {
                values[i] = value;
                return true;
            }
        }

        if (size >= CAPACITY) {
            return false;
        }

        keys[size] = key;
        values[size] = value;
        ++size;
        return true;
    }

    // Tìm value theo key. Kết quả được ghi vào biến value.
    bool find(const string& key, int& value) const {
        for (int i = 0; i < size; ++i) {
            if (keys[i] == key) {
                value = values[i];
                return true;
            }
        }
        return false;
    }

    // Xóa một cặp key-value và dồn các phần tử phía sau lên.
    bool remove(const string& key) {
        for (int i = 0; i < size; ++i) {
            if (keys[i] == key) {
                for (int j = i; j < size - 1; ++j) {
                    keys[j] = keys[j + 1];
                    values[j] = values[j + 1];
                }

                --size;
                return true;
            }
        }
        return false;
    }

    bool contains(const string& key) const {
        for (int i = 0; i < size; ++i) {
            if (keys[i] == key) {
                return true;
            }
        }
        return false;
    }

    int getSize() const {
        return size;
    }

    int getCapacity() const {
        return CAPACITY;
    }

    bool isEmpty() const {
        return size == 0;
    }

    void display() const {
        cout << "Map: ";

        if (size == 0) {
            cout << "(rong)";
        } else {
            for (int i = 0; i < size; ++i) {
                cout << "[" << keys[i] << " -> " << values[i] << "] ";
            }
        }

        cout << endl;
    }
};


// ============================================================
// 5. BINARY SEARCH TREE CHO INT
// ============================================================

// Node của cây tìm kiếm nhị phân.
struct BSTNodeInt {
    int data;
    BSTNodeInt* left;
    BSTNodeInt* right;

    explicit BSTNodeInt(int val)
        : data(val), left(nullptr), right(nullptr) {}
};

// BST: các giá trị nhỏ hơn node nằm bên trái,
// các giá trị lớn hơn node nằm bên phải.
// Giá trị trùng nhau không được chèn thêm.
class BinarySearchTreeInt {
private:
    BSTNodeInt* root;
    int size;

    BSTNodeInt* insertRecursive(BSTNodeInt* node, int val) {
        if (node == nullptr) {
            BSTNodeInt* newNode = new BSTNodeInt(val);
            ++size;
            return newNode;
        }

        if (val < node->data) {
            node->left = insertRecursive(node->left, val);
        } else if (val > node->data) {
            node->right = insertRecursive(node->right, val);
        }

        return node;
    }

    BSTNodeInt* searchRecursive(BSTNodeInt* node, int val) const {
        if (node == nullptr || node->data == val) {
            return node;
        }

        if (val < node->data) {
            return searchRecursive(node->left, val);
        }

        return searchRecursive(node->right, val);
    }

    BSTNodeInt* minValueNode(BSTNodeInt* node) const {
        while (node != nullptr && node->left != nullptr) {
            node = node->left;
        }
        return node;
    }

    // Xóa node và báo qua biến removed xem có xóa được hay không.
    // Việc giảm size được thực hiện duy nhất một lần trong remove().
    BSTNodeInt* deleteRecursive(
        BSTNodeInt* node,
        int val,
        bool& removed
    ) {
        if (node == nullptr) {
            return nullptr;
        }

        if (val < node->data) {
            node->left = deleteRecursive(node->left, val, removed);
        } else if (val > node->data) {
            node->right = deleteRecursive(node->right, val, removed);
        } else {
            // Trường hợp node không có con trái.
            if (node->left == nullptr) {
                BSTNodeInt* child = node->right;
                delete node;
                removed = true;
                return child;
            }

            // Trường hợp node không có con phải.
            if (node->right == nullptr) {
                BSTNodeInt* child = node->left;
                delete node;
                removed = true;
                return child;
            }

            // Trường hợp node có cả hai con:
            // thay dữ liệu bằng node nhỏ nhất của cây con phải,
            // sau đó xóa node kế nhiệm cũ.
            BSTNodeInt* successor = minValueNode(node->right);
            int successorValue = successor->data;
            node->data = successorValue;

            bool successorRemoved = false;
            node->right = deleteRecursive(
                node->right,
                successorValue,
                successorRemoved
            );

            removed = true;
        }

        return node;
    }

    void inorderRecursive(
        BSTNodeInt* node,
        int* result,
        int& index
    ) const {
        if (node == nullptr) {
            return;
        }

        inorderRecursive(node->left, result, index);
        result[index++] = node->data;
        inorderRecursive(node->right, result, index);
    }

    void inorderRecursive(
        BSTNodeInt* node,
        vector<int>& result
    ) const {
        if (node == nullptr) {
            return;
        }

        inorderRecursive(node->left, result);
        result.push_back(node->data);
        inorderRecursive(node->right, result);
    }

    BSTNodeInt* cloneTree(const BSTNodeInt* node) {
        if (node == nullptr) {
            return nullptr;
        }

        BSTNodeInt* newNode = new BSTNodeInt(node->data);

        try {
            newNode->left = cloneTree(node->left);
            newNode->right = cloneTree(node->right);
        } catch (...) {
            destroyTree(newNode);
            throw;
        }

        return newNode;
    }

    void destroyTree(BSTNodeInt* node) {
        if (node == nullptr) {
            return;
        }

        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }

    void displayHelper(BSTNodeInt* node, int level = 0) const {
        if (node == nullptr) {
            return;
        }

        displayHelper(node->right, level + 1);

        for (int i = 0; i < level; ++i) {
            cout << "    ";
        }

        cout << node->data << endl;
        displayHelper(node->left, level + 1);
    }

public:
    BinarySearchTreeInt() : root(nullptr), size(0) {}

    // Sao chép sâu cây để tránh hai đối tượng cùng sở hữu một cây.
    BinarySearchTreeInt(const BinarySearchTreeInt& other)
        : root(nullptr), size(other.size) {
        root = cloneTree(other.root);
    }

    BinarySearchTreeInt& operator=(const BinarySearchTreeInt& other) {
        if (this != &other) {
            BinarySearchTreeInt temp(other);
            swap(temp);
        }
        return *this;
    }

    BinarySearchTreeInt(BinarySearchTreeInt&& other) noexcept
        : root(other.root), size(other.size) {
        other.root = nullptr;
        other.size = 0;
    }

    BinarySearchTreeInt& operator=(BinarySearchTreeInt&& other) noexcept {
        if (this != &other) {
            destroyTree(root);

            root = other.root;
            size = other.size;

            other.root = nullptr;
            other.size = 0;
        }
        return *this;
    }

    ~BinarySearchTreeInt() {
        destroyTree(root);
    }

    void swap(BinarySearchTreeInt& other) noexcept {
        std::swap(root, other.root);
        std::swap(size, other.size);
    }

    void insert(int val) {
        root = insertRecursive(root, val);
    }

    bool contains(int val) const {
        return searchRecursive(root, val) != nullptr;
    }

    // Xóa đúng một node. Với node có hai con, size vẫn chỉ giảm 1.
    bool remove(int val) {
        bool removed = false;
        root = deleteRecursive(root, val, removed);

        if (removed) {
            --size;
        }

        return removed;
    }

    int getSize() const {
        return size;
    }

    bool isEmpty() const {
        return root == nullptr;
    }

    // Lưu kết quả inorder vào mảng int*.
    // Người gọi phải cấp phát mảng có ít nhất getSize() phần tử.
    void inorder(int* result) const {
        if (result == nullptr) {
            return;
        }

        int index = 0;
        inorderRecursive(root, result, index);
    }

    // Phiên bản trả về vector, tự quản lý kích thước bộ nhớ.
    vector<int> inorder() const {
        vector<int> result;
        result.reserve(size);
        inorderRecursive(root, result);
        return result;
    }

    void display() const {
        cout << "BST (inorder - thứ tự tăng dần): ";

        vector<int> sortedValues = inorder();
        if (sortedValues.empty()) {
            cout << "(rong)";
        } else {
            for (size_t i = 0; i < sortedValues.size(); ++i) {
                cout << sortedValues[i] << " ";
            }
        }

        cout << "\nCấu trúc cây (nhánh phải nằm trên):" << endl;

        if (root == nullptr) {
            cout << "(cay rong)" << endl;
        } else {
            displayHelper(root);
        }
    }
};


// ============================================================
// 6. SEARCH ALGORITHMS - CAC THUAT TOAN TIM KIEM
// ============================================================

// Tìm kiếm tuyến tính.
// Trả về chỉ số tìm thấy đầu tiên hoặc -1 nếu không tìm thấy.
int linearSearchString(
    const string* arr,
    int n,
    const string& target
) {
    if (arr == nullptr || n <= 0) {
        return -1;
    }

    for (int i = 0; i < n; ++i) {
        if (arr[i] == target) {
            return i;
        }
    }

    return -1;
}

// Tìm kiếm nhị phân trên mảng string ĐÃ ĐƯỢC SẮP XẾP TĂNG DẦN.
// Trả về chỉ số tìm thấy hoặc -1 nếu không tìm thấy.
int binarySearchString(
    const string* arr,
    int n,
    const string& target
) {
    if (arr == nullptr || n <= 0) {
        return -1;
    }

    int left = 0;
    int right = n - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (arr[mid] == target) {
            return mid;
        }

        if (arr[mid] < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return -1;
}


// ============================================================
// 7. SORTING ALGORITHMS - CAC THUAT TOAN SAP XEP
// ============================================================

// Bubble Sort cho mảng string theo thứ tự từ điển tăng dần.
void bubbleSortString(string* arr, int n) {
    if (arr == nullptr || n <= 1) {
        return;
    }

    for (int i = 0; i < n - 1; ++i) {
        bool swapped = false;

        for (int j = 0; j < n - i - 1; ++j) {
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);
                swapped = true;
            }
        }

        // Nếu một lượt không có hoán đổi thì mảng đã được sắp xếp.
        if (!swapped) {
            break;
        }
    }
}

// Bubble Sort cho mảng int theo thứ tự tăng dần.
void bubbleSortInt(int* arr, int n) {
    if (arr == nullptr || n <= 1) {
        return;
    }

    for (int i = 0; i < n - 1; ++i) {
        bool swapped = false;

        for (int j = 0; j < n - i - 1; ++j) {
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);
                swapped = true;
            }
        }

        if (!swapped) {
            break;
        }
    }
}

// Quick Sort cho mảng int.
// low và high là chỉ số đầu và cuối, tính cả hai đầu.
// Ví dụ: quickSortInt(arr, 0, n - 1).
// Chọn pivot ở giữa và xử lý nhánh nhỏ trước để hạn chế độ sâu đệ quy.
void quickSortInt(int* arr, int low, int high) {
    if (arr == nullptr || low < 0 || low >= high) {
        return;
    }

    while (low < high) {
        int i = low;
        int j = high;
        int pivot = arr[low + (high - low) / 2];

        while (i <= j) {
            while (arr[i] < pivot) {
                ++i;
            }

            while (arr[j] > pivot) {
                --j;
            }

            if (i <= j) {
                std::swap(arr[i], arr[j]);
                ++i;
                --j;
            }
        }

        // Đệ quy nhánh ngắn hơn, lặp tiếp với nhánh dài hơn.
        if (j - low < high - i) {
            if (low < j) {
                quickSortInt(arr, low, j);
            }
            low = i;
        } else {
            if (i < high) {
                quickSortInt(arr, i, high);
            }
            high = j;
        }
    }
}


// ============================================================
// 8. SAP XEP DANH SACH SAN PHAM CUA BOARD_AND_BOOK
// ============================================================

// Các tiêu chí sắp xếp được dùng chung cho Bubble Sort và Quick Sort.
enum ProductSortCriteria {
    PRODUCT_SORT_NAME_ASC,          // Tên A -> Z
    PRODUCT_SORT_ID_ASC,            // Mã sản phẩm A -> Z
    PRODUCT_SORT_QUANTITY_ASC,      // Số lượng ít -> nhiều
    PRODUCT_SORT_QUANTITY_DESC,     // Số lượng nhiều -> ít
    PRODUCT_SORT_STATUS_ASC,        // AVAILABLE -> RENTED -> DAMAGED
    PRODUCT_SORT_RENTAL_PRICE_ASC   // Giá thuê tăng dần; chỉ dùng cho cùng một loại sản phẩm
};

// Kiểm tra xem a có nên đứng sau b theo tiêu chí đã chọn hay không.
// Hàm này chỉ so sánh, không thay đổi dữ liệu của sản phẩm.
inline bool productComesAfter(
    const Product* a,
    const Product* b,
    ProductSortCriteria criteria
) {
    // Con trỏ null (nếu có) được đẩy về cuối danh sách.
    if (a == nullptr) return b != nullptr;
    if (b == nullptr) return false;

    switch (criteria) {
        case PRODUCT_SORT_NAME_ASC:
            return a->getName() > b->getName();

        case PRODUCT_SORT_ID_ASC:
            return a->getProductID() > b->getProductID();

        case PRODUCT_SORT_QUANTITY_ASC:
            return a->getQuantity() > b->getQuantity();

        case PRODUCT_SORT_QUANTITY_DESC:
            return a->getQuantity() < b->getQuantity();

        case PRODUCT_SORT_STATUS_ASC:
            return static_cast<int>(a->getStatus()) >
                   static_cast<int>(b->getStatus());

        case PRODUCT_SORT_RENTAL_PRICE_ASC:
            // Chỉ gọi tiêu chí này sau khi đã lọc riêng ComicBook hoặc BoardGame,
            // vì một bên tính theo tháng còn một bên tính theo giờ.
            return a->calculateRentalFee(1) > b->calculateRentalFee(1);
    }

    return false;
}

// Bubble Sort dành cho danh sách con trỏ Product*.
// Sắp xếp trực tiếp vector được truyền vào.
void bubbleSortProducts(
    vector<Product*>& products,
    ProductSortCriteria criteria
) {
    const size_t n = products.size();
    if (n <= 1) return;

    for (size_t i = 0; i < n - 1; ++i) {
        bool swapped = false;

        for (size_t j = 0; j < n - i - 1; ++j) {
            if (productComesAfter(products[j], products[j + 1], criteria)) {
                std::swap(products[j], products[j + 1]);
                swapped = true;
            }
        }

        // Không còn hoán đổi nghĩa là danh sách đã đúng thứ tự.
        if (!swapped) break;
    }
}

// Hàm phụ trợ của Quick Sort: chia danh sách quanh pivot.
int partitionProducts(
    vector<Product*>& products,
    int low,
    int high,
    ProductSortCriteria criteria
) {
    Product* pivot = products[low + (high - low) / 2];
    int i = low;
    int j = high;

    while (i <= j) {
        while (i <= high && productComesAfter(pivot, products[i], criteria)) {
            ++i;
        }
        while (j >= low && productComesAfter(products[j], pivot, criteria)) {
            --j;
        }

        if (i <= j) {
            std::swap(products[i], products[j]);
            ++i;
            --j;
        }
    }

    return i;
}

// Quick Sort đệ quy trên đoạn [low, high].
void quickSortProducts(
    vector<Product*>& products,
    int low,
    int high,
    ProductSortCriteria criteria
) {
    if (products.empty() || low < 0 || high < 0 || low >= high ||
        high >= static_cast<int>(products.size())) {
        return;
    }

    int index = partitionProducts(products, low, high, criteria);

    if (low < index - 1) {
        quickSortProducts(products, low, index - 1, criteria);
    }
    if (index < high) {
        quickSortProducts(products, index, high, criteria);
    }
}

// Phiên bản Quick Sort tự xác định đoạn cần sắp xếp.
void quickSortProducts(
    vector<Product*>& products,
    ProductSortCriteria criteria
) {
    if (products.size() > 1) {
        quickSortProducts(
            products,
            0,
            static_cast<int>(products.size()) - 1,
            criteria
        );
    }
}

// Lọc riêng các truyện tranh để không so sánh giá thuê theo tháng
// với giá thuê board game theo giờ.
vector<Product*> getComicBooks(const vector<Product*>& products) {
    vector<Product*> result;
    for (size_t i = 0; i < products.size(); ++i) {
        if (dynamic_cast<ComicBook*>(products[i]) != nullptr) {
            result.push_back(products[i]);
        }
    }
    return result;
}

// Lọc riêng các board game.
vector<Product*> getBoardGames(const vector<Product*>& products) {
    vector<Product*> result;
    for (size_t i = 0; i < products.size(); ++i) {
        if (dynamic_cast<BoardGame*>(products[i]) != nullptr) {
            result.push_back(products[i]);
        }
    }
    return result;
}

#endif // BOARD_AND_BOOK_DATA_STRUCTURES_CPP
