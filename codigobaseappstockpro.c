#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <string.h>

// Asegúrate de enlazar winmm.lib en las opciones de tu proyecto.

#define ID_EDIT_NAME      1
#define ID_EDIT_PRICE     2
#define ID_EDIT_QUANTITY  3
#define ID_BUTTON_ADD     4
#define ID_BUTTON_SEARCH  5
#define ID_BUTTON_EDIT    6
#define ID_BUTTON_DELETE  7
#define ID_OUTPUT         8
#define ID_LISTBOX        9
#define ID_COLOR_COMBOBOX 100

HWND hEditName, hEditPrice, hEditQuantity, hOutput, hListBox, hColorCombo;
HBITMAP hBackgroundImage = NULL;

// Pinceles para cada tema
HBRUSH hbrBackWhite,  hbrBackGray,   hbrBackLightBlue, hbrBackDark;
COLORREF crTextWhite, crTextGray,    crTextLightBlue,   crTextDark;
// Tema actual: 0=Blanco, 1=Gris, 2=Azul Claro, 3=Oscuro
int temaActual = 0;

typedef struct {
    char name[100];
    float price;
    int quantity;
} Product;

int ProductExists(const char* name) {
    FILE* file = fopen("productos.txt", "r");
    if (!file) return 0;
    char prodName[100];
    float price;
    int quantity;
    while (fscanf(file, "%s %f %d", prodName, &price, &quantity) == 3) {
        if (strcmp(prodName, name) == 0) {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

void AddProduct(const char* name, float price, int quantity) {
    if (ProductExists(name)) {
        MessageBox(NULL, "Producto ya existe.", "Error", MB_ICONERROR);
        return;
    }
    FILE* file = fopen("productos.txt", "a");
    if (file) {
        fprintf(file, "%s %.2f %d\n", name, price, quantity);
        fclose(file);
    }
}

int SearchProduct(const char* name, char* result, int maxLen) {
    FILE* file = fopen("productos.txt", "r");
    char prodName[100];
    float price;
    int quantity;

    if (!file) return 0;

    while (fscanf(file, "%s %f %d", prodName, &price, &quantity) == 3) {
        if (strcmp(name, prodName) == 0) {
            snprintf(result, maxLen, "Nombre: %s\nPrecio: %.2f\nCantidad: %d", prodName, price, quantity);
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

int EditProduct(const char* name, float newPrice, int newQuantity) {
    FILE* file = fopen("productos.txt", "r");
    if (!file) return 0;

    Product products[100];
    int count = 0, found = 0;

    while (fscanf(file, "%s %f %d", products[count].name, &products[count].price, &products[count].quantity) == 3) {
        if (strcmp(products[count].name, name) == 0) {
            products[count].price = newPrice;
            products[count].quantity = newQuantity;
            found = 1;
        }
        count++;
    }
    fclose(file);

    if (!found) return 0;

    file = fopen("productos.txt", "w");
    for (int i = 0; i < count; i++) {
        fprintf(file, "%s %.2f %d\n", products[i].name, products[i].price, products[i].quantity);
    }
    fclose(file);
    return 1;
}

int DeleteProduct(const char* name) {
    FILE* file = fopen("productos.txt", "r");
    if (!file) return 0;

    Product products[100];
    int count = 0, found = 0;

    while (fscanf(file, "%s %f %d", products[count].name, &products[count].price, &products[count].quantity) == 3) {
        if (strcmp(products[count].name, name) == 0) {
            found = 1;
        } else {
            count++;
        }
    }
    fclose(file);

    if (!found) return 0;

    file = fopen("productos.txt", "w");
    for (int i = 0; i < count; i++) {
        fprintf(file, "%s %.2f %d\n", products[i].name, products[i].price, products[i].quantity);
    }
    fclose(file);
    return 1;
}

void LoadProductsToListBox() {
    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
    FILE* file = fopen("productos.txt", "r");
    if (!file) return;

    char name[100];
    float price;
    int quantity;
    char buffer[256];

    while (fscanf(file, "%s %f %d", name, &price, &quantity) == 3) {
        snprintf(buffer, sizeof(buffer), "%s | Precio: %.2f | Cantidad: %d", name, price, quantity);
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)buffer);
    }
    fclose(file);
}

void ResizeControls(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int width = rc.right;
    int height = rc.bottom;

    int editLeft = 80;
    int editWidth = width - editLeft - 20;
    int y = 10;
    int spacing = 30;

    MoveWindow(hEditName,       editLeft, y, editWidth, 20, TRUE);
    y += spacing;
    MoveWindow(hEditPrice,      editLeft, y, editWidth, 20, TRUE);
    y += spacing;
    MoveWindow(hEditQuantity,   editLeft, y, editWidth, 20, TRUE);
    y += spacing;

    MoveWindow(GetDlgItem(hwnd, ID_BUTTON_ADD),    10, y, width / 2 - 15, 25, TRUE);
    MoveWindow(GetDlgItem(hwnd, ID_BUTTON_SEARCH), width / 2 + 5, y, width / 2 - 15, 25, TRUE);
    y += 30;
    MoveWindow(GetDlgItem(hwnd, ID_BUTTON_EDIT),   10, y, width - 20, 25, TRUE);
    y += 30;
    MoveWindow(GetDlgItem(hwnd, ID_BUTTON_DELETE), 10, y, width - 20, 25, TRUE);
    y += 30;

    MoveWindow(hOutput, 10, y, width - 20, 40, TRUE);
    y += 50;

    // Tamaño original del ListBox
    int originalListWidth = width - 20;
    int originalListHeight = height - y - 70;

    // Reducir ancho y alto en un 20%
    int newListWidth  = (int)(originalListWidth * 0.8f);
    int newListHeight = (int)(originalListHeight * 0.8f);

    MoveWindow(hListBox, 10, y, newListWidth, newListHeight, TRUE);

    // Ajustar posición del ComboBox de temas (manteniéndolo 50px desde abajo)
    MoveWindow(GetDlgItem(hwnd, ID_COLOR_COMBOBOX), 60, height - 50, 120, 100, TRUE);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char name[100], priceStr[100], quantityStr[100];
    float price;
    int quantity;
    char result[256];

    switch (uMsg) {
    case WM_CREATE:
        // Crear controles
        CreateWindow("STATIC", "Nombre:", WS_VISIBLE | WS_CHILD, 10, 10, 60, 20, hwnd, NULL, NULL, NULL);
        hEditName = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 80, 10, 200, 20, hwnd, (HMENU)ID_EDIT_NAME, NULL, NULL);

        CreateWindow("STATIC", "Precio:", WS_VISIBLE | WS_CHILD, 10, 40, 60, 20, hwnd, NULL, NULL, NULL);
        hEditPrice = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 80, 40, 200, 20, hwnd, (HMENU)ID_EDIT_PRICE, NULL, NULL);

        CreateWindow("STATIC", "Cantidad:", WS_VISIBLE | WS_CHILD, 10, 70, 60, 20, hwnd, NULL, NULL, NULL);
        hEditQuantity = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 80, 70, 200, 20, hwnd, (HMENU)ID_EDIT_QUANTITY, NULL, NULL);

        CreateWindow("BUTTON", "Cargar Producto", WS_VISIBLE | WS_CHILD, 10, 100, 130, 25, hwnd, (HMENU)ID_BUTTON_ADD, NULL, NULL);
        CreateWindow("BUTTON", "Buscar Producto", WS_VISIBLE | WS_CHILD, 150, 100, 130, 25, hwnd, (HMENU)ID_BUTTON_SEARCH, NULL, NULL);
        CreateWindow("BUTTON", "Editar Producto", WS_VISIBLE | WS_CHILD, 10, 130, 270, 25, hwnd, (HMENU)ID_BUTTON_EDIT, NULL, NULL);
        CreateWindow("BUTTON", "Eliminar Producto", WS_VISIBLE | WS_CHILD, 10, 160, 270, 25, hwnd, (HMENU)ID_BUTTON_DELETE, NULL, NULL);

        hOutput = CreateWindow("STATIC", "", WS_VISIBLE | WS_CHILD | SS_LEFT, 10, 190, 270, 40, hwnd, (HMENU)ID_OUTPUT, NULL, NULL);
        hListBox = CreateWindow("LISTBOX", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY, 10, 240, 270, 100, hwnd, (HMENU)ID_LISTBOX, NULL, NULL);

        CreateWindow("STATIC", "Tema:", WS_VISIBLE | WS_CHILD, 10, 350, 40, 20, hwnd, NULL, NULL, NULL);
        hColorCombo = CreateWindow("COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
                                   60, 350, 120, 100, hwnd, (HMENU)ID_COLOR_COMBOBOX, NULL, NULL);
        SendMessage(hColorCombo, CB_ADDSTRING, 0, (LPARAM)"Blanco");
        SendMessage(hColorCombo, CB_ADDSTRING, 0, (LPARAM)"Gris");
        SendMessage(hColorCombo, CB_ADDSTRING, 0, (LPARAM)"Azul Claro");
        SendMessage(hColorCombo, CB_ADDSTRING, 0, (LPARAM)"Oscuro");
        SendMessage(hColorCombo, CB_SETCURSEL, 0, 0);

        // Cargar imagen de fondo
        hBackgroundImage = (HBITMAP)LoadImage(NULL, "logo.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        if (!hBackgroundImage) {
            MessageBox(hwnd, "No se pudo cargar logo.bmp", "Error", MB_ICONERROR);
        }

        // Crear pinceles y colores de texto para cada tema
        hbrBackWhite     = CreateSolidBrush(RGB(255,255,255));
        hbrBackGray      = CreateSolidBrush(RGB(200,200,200));
        hbrBackLightBlue = CreateSolidBrush(RGB(220,230,255));
        hbrBackDark      = CreateSolidBrush(RGB( 45, 45, 48));

        crTextWhite     = RGB(0,0,0);
        crTextGray      = RGB(0,0,0);
        crTextLightBlue = RGB(0,0,0);
        crTextDark      = RGB(240,240,240);

        temaActual = 0;
        LoadProductsToListBox();
        break;

    case WM_SIZE:
        ResizeControls(hwnd);
        break;

    case WM_COMMAND:
        // Obtener textos de los edits
        GetWindowText(hEditName, name, sizeof(name));
        GetWindowText(hEditPrice, priceStr, sizeof(priceStr));
        GetWindowText(hEditQuantity, quantityStr, sizeof(quantityStr));
        price = atof(priceStr);
        quantity = atoi(quantityStr);

        switch (LOWORD(wParam)) {
        case ID_BUTTON_ADD:
            AddProduct(name, price, quantity);
            SetWindowText(hOutput, "Producto guardado.");
            LoadProductsToListBox();
            PlaySound(TEXT("click.wav"), NULL, SND_FILENAME | SND_ASYNC);
            break;

        case ID_BUTTON_SEARCH:
            if (SearchProduct(name, result, sizeof(result))) {
                SetWindowText(hOutput, result);
                PlaySound(TEXT("click.wav"), NULL, SND_FILENAME | SND_ASYNC);
            } else {
                SetWindowText(hOutput, "Producto no encontrado.");
                PlaySound(TEXT("error.wav"), NULL, SND_FILENAME | SND_ASYNC);
            }
            break;

        case ID_BUTTON_EDIT:
            if (EditProduct(name, price, quantity)) {
                SetWindowText(hOutput, "Producto actualizado.");
                LoadProductsToListBox();
                PlaySound(TEXT("click.wav"), NULL, SND_FILENAME | SND_ASYNC);
            } else {
                SetWindowText(hOutput, "Producto no encontrado.");
                PlaySound(TEXT("error.wav"), NULL, SND_FILENAME | SND_ASYNC);
            }
            break;

        case ID_BUTTON_DELETE:
            if (DeleteProduct(name)) {
                SetWindowText(hOutput, "Producto eliminado.");
                LoadProductsToListBox();
                PlaySound(TEXT("click.wav"), NULL, SND_FILENAME | SND_ASYNC);
            } else {
                SetWindowText(hOutput, "Producto no encontrado.");
                PlaySound(TEXT("error.wav"), NULL, SND_FILENAME | SND_ASYNC);
            }
            break;

        case ID_LISTBOX:
            if (HIWORD(wParam) == LBN_SELCHANGE) {
                int selIndex = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                if (selIndex != LB_ERR) {
                    char buffer[256];
                    SendMessage(hListBox, LB_GETTEXT, selIndex, (LPARAM)buffer);
                    sscanf(buffer, "%s", name);
                    SetWindowText(hEditName, name);
                }
            }
            break;

        case ID_COLOR_COMBOBOX:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                temaActual = SendMessage(hColorCombo, CB_GETCURSEL, 0, 0);
                InvalidateRect(hwnd, NULL, TRUE);
                UpdateWindow(hwnd);
            }
            break;
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;
        GetClientRect(hwnd, &rc);

        if (temaActual == 3) {
            // Modo Oscuro: fondo liso
            FillRect(hdc, &rc, hbrBackDark);
        } else {
            // Otros temas: imagen de fondo o pincel
            if (hBackgroundImage) {
                HDC hdcMem = CreateCompatibleDC(hdc);
                HBITMAP oldBmp = (HBITMAP)SelectObject(hdcMem, hBackgroundImage);
                BITMAP bmp;
                GetObject(hBackgroundImage, sizeof(BITMAP), &bmp);
                StretchBlt(hdc, 0, 0, rc.right, rc.bottom,
                           hdcMem, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
                SelectObject(hdcMem, oldBmp);
                DeleteDC(hdcMem);
            } else {
                HBRUSH hbr =
                    (temaActual == 0) ? hbrBackWhite :
                    (temaActual == 1) ? hbrBackGray :
                    hbrBackLightBlue;
                FillRect(hdc, &rc, hbr);
            }
        }

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORBTN: {
        HDC hdcCtrl = (HDC)wParam;
        switch (temaActual) {
            case 0:
                SetTextColor(hdcCtrl, crTextWhite);
                SetBkColor(hdcCtrl, RGB(255,255,255));
                return (INT_PTR)hbrBackWhite;
            case 1:
                SetTextColor(hdcCtrl, crTextGray);
                SetBkColor(hdcCtrl, RGB(200,200,200));
                return (INT_PTR)hbrBackGray;
            case 2:
                SetTextColor(hdcCtrl, crTextLightBlue);
                SetBkColor(hdcCtrl, RGB(220,230,255));
                return (INT_PTR)hbrBackLightBlue;
            case 3:
                SetTextColor(hdcCtrl, crTextDark);
                SetBkColor(hdcCtrl, RGB(45,45,48));
                return (INT_PTR)hbrBackDark;
        }
        break;
    }

    case WM_DESTROY:
        // Liberar pinceles e imagen de fondo
        DeleteObject(hbrBackWhite);
        DeleteObject(hbrBackGray);
        DeleteObject(hbrBackLightBlue);
        DeleteObject(hbrBackDark);
        if (hBackgroundImage) DeleteObject(hBackgroundImage);
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "ProductWindowClass";
    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = NULL;        // Dejar que WM_PAINT pinte el fondo
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Gestor de Productos",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 500,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
