#include <iostream>
#include <fstream>
#include <cstring>
#include <queue>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cctype>

using namespace std;

// ==================== 位图 Bitmap 类 ====================
class Bitmap {
private:
    unsigned char* M;
    int N; // 字节数
    int _sz; // 有效位数

protected:
    void init(int n) {
        N = (n + 7) / 8;
        M = new unsigned char[N];
        memset(M, 0, N * sizeof(unsigned char));
        _sz = 0;
    }

public:
    Bitmap(int n = 8) { init(n); }
    Bitmap(const Bitmap& other) {
        init(other.N * 8);
        memcpy(M, other.M, N);
        _sz = other._sz;
    }
    
    Bitmap& operator=(const Bitmap& other) {
        if (this != &other) {
            delete[] M;
            init(other.N * 8);
            memcpy(M, other.M, N);
            _sz = other._sz;
        }
        return *this;
    }
    
    ~Bitmap() { delete[] M; M = nullptr; _sz = 0; }

    int size() const { return _sz; }
    int capacity() const { return N * 8; }

    void set(int k) {
        expand(k);
        if (!test(k)) {
            _sz++;
            M[k >> 3] |= (0x80 >> (k & 0x07));
        }
    }

    void clear(int k) {
        expand(k);
        if (test(k)) {
            _sz--;
            M[k >> 3] &= ~(0x80 >> (k & 0x07));
        }
    }

    bool test(int k) const {
        if (k >= N * 8) return false;
        return (M[k >> 3] & (0x80 >> (k & 0x07))) != 0;
    }

    std::string bits2string(int n) const {
        if (n > N * 8) n = N * 8;
        std::string s;
        s.reserve(n);
        for (int i = 0; i < n; i++) {
            s.push_back(test(i) ? '1' : '0');
        }
        return s;
    }

    void expand(int k) {
        if (k < N * 8) return;
        int newN = (k + 8) / 8; // 多分配一个字节
        unsigned char* newM = new unsigned char[newN];
        memset(newM, 0, newN * sizeof(unsigned char));
        memcpy(newM, M, N);
        delete[] M;
        M = newM;
        N = newN;
    }
    
    // 获取统计信息
    void printStats() const {
        cout << "Bitmap Stats: " << endl;
        cout << "  Capacity: " << capacity() << " bits" << endl;
        cout << "  Size: " << size() << " bits set" << endl;
        cout << "  Utilization: " << (size() * 100.0 / capacity()) << "%" << endl;
    }
};

// ==================== 二叉树节点结构 ====================
struct BinNode {
    char data; // 字符
    int freq; // 频率
    BinNode* left;
    BinNode* right;

    BinNode(char d, int f) : data(d), freq(f), left(nullptr), right(nullptr) {}
    bool isLeaf() const { return !left && !right; }
};

// ==================== 二叉树 BinTree 类 ====================
class BinTree {
private:
    BinNode* root;
public:
    BinTree() : root(nullptr) {}
    BinTree(BinNode* r) : root(r) {}
    ~BinTree() { clear(root); }

    BinNode* getRoot() const { return root; }
    
    // 支持移动语义
    BinTree(BinTree&& other) noexcept : root(other.root) {
        other.root = nullptr;
    }
    
    BinTree& operator=(BinTree&& other) noexcept {
        if (this != &other) {
            clear(root);
            root = other.root;
            other.root = nullptr;
        }
        return *this;
    }
    
    // 禁止拷贝构造和赋值
    BinTree(const BinTree&) = delete;
    BinTree& operator=(const BinTree&) = delete;

    void clear(BinNode* node) {
        if (node) {
            clear(node->left);
            clear(node->right);
            delete node;
        }
        root = nullptr;
    }
    
    // 计算树的高度
    int height(BinNode* node) const {
        if (!node) return 0;
        return 1 + max(height(node->left), height(node->right));
    }
    
    int height() const {
        return height(root);
    }
    
    // 计算树的大小（节点数）
    int size(BinNode* node) const {
        if (!node) return 0;
        return 1 + size(node->left) + size(node->right);
    }
    
    int size() const {
        return size(root);
    }
};

// ==================== 优先队列比较函数 ====================
struct Compare {
    bool operator()(BinNode* a, BinNode* b) {
        // 频率相同则按字符排序，确保编码的一致性
        if (a->freq == b->freq) {
            return a->data > b->data;
        }
        return a->freq > b->freq;
    }
};

// ==================== Huffman 树构建类 ====================
class HuffTree {
private:
    BinTree tree;
    unordered_map<char, string> codeMap;
    unordered_map<string, char> reverseCodeMap; // 反向查找表，用于解码
    int totalEncodedBits;

    void generateCodes(BinNode* node, const string& code) {
        if (!node) return;
        if (node->isLeaf()) {
            codeMap[node->data] = code;
            reverseCodeMap[code] = node->data;
        } else {
            generateCodes(node->left, code + "0");
            generateCodes(node->right, code + "1");
        }
    }

public:
    HuffTree() : totalEncodedBits(0) {}
    ~HuffTree() {}

    void buildFromText(const string& text) {
        // 清空之前的映射
        codeMap.clear();
        reverseCodeMap.clear();
        totalEncodedBits = 0;
        
        // 统计频率（只保留字母，忽略大小写）
        unordered_map<char, int> freqMap;
        int totalChars = 0;
        for (char ch : text) {
            if (isalpha(ch)) {
                char lowerCh = tolower(ch);
                freqMap[lowerCh]++;
                totalChars++;
            }
        }
        
        cout << "Character frequency statistics:" << endl;
        cout << "Total alphabetic characters: " << totalChars << endl;
        cout << "Unique characters: " << freqMap.size() << endl;

        // 构建最小堆
        priority_queue<BinNode*, vector<BinNode*>, Compare> pq;
        for (const auto& pair : freqMap) {
            pq.push(new BinNode(pair.first, pair.second));
        }
        
        // 特殊情况：只有一个字符
        if (pq.size() == 1) {
            BinNode* single = pq.top();
            tree = BinTree(new BinNode('\0', single->freq));
            tree.getRoot()->left = single;
            generateCodes(tree.getRoot(), "");
            return;
        }

        // 构建 Huffman 树
        while (pq.size() > 1) {
            BinNode* left = pq.top(); pq.pop();
            BinNode* right = pq.top(); pq.pop();
            BinNode* parent = new BinNode('\0', left->freq + right->freq);
            parent->left = left;
            parent->right = right;
            pq.push(parent);
        }

        if (!pq.empty()) {
            tree = BinTree(pq.top());
            generateCodes(tree.getRoot(), "");
        }
        
        // 计算总编码位数
        for (const auto& pair : freqMap) {
            totalEncodedBits += pair.second * codeMap[pair.first].length();
        }
    }

    string encode(const string& word) {
        string encoded = "";
        for (char ch : word) {
            if (isalpha(ch)) {
                char lowerCh = tolower(ch);
                if (codeMap.find(lowerCh) != codeMap.end()) {
                    encoded += codeMap[lowerCh];
                }
            }
        }
        return encoded;
    }
    
    string decode(const string& binaryStr) {
        string decoded = "";
        string currentCode = "";
        
        for (char bit : binaryStr) {
            currentCode += bit;
            if (reverseCodeMap.find(currentCode) != reverseCodeMap.end()) {
                decoded += reverseCodeMap[currentCode];
                currentCode = "";
            }
        }
        
        return decoded;
    }

    void printCodes() const {
        cout << "\nHuffman Codes (sorted by frequency):" << endl;
        cout << "======================================" << endl;
        
        // 将编码按长度排序，相同长度按字母排序
        vector<pair<char, string>> sortedCodes(codeMap.begin(), codeMap.end());
        sort(sortedCodes.begin(), sortedCodes.end(), 
             [](const pair<char, string>& a, const pair<char, string>& b) {
                 if (a.second.length() != b.second.length())
                     return a.second.length() < b.second.length();
                 if (a.first != b.first)
                     return a.first < b.first;
                 return a.second < b.second;
             });
        
        for (const auto& pair : sortedCodes) {
            cout << "'" << pair.first << "' : " << pair.second 
                 << " (length: " << pair.second.length() << ")" << endl;
        }
        
        cout << "\nTree Statistics:" << endl;
        cout << "Tree height: " << tree.height() << endl;
        cout << "Tree nodes: " << tree.size() << endl;
        cout << "Total encoded bits: " << totalEncodedBits << endl;
        
        // 计算压缩率
        int originalBits = 0;
        for (const auto& pair : codeMap) {
            originalBits += pair.second.length() * 8; // 假设每个字符8位
        }
        if (originalBits > 0) {
            double compressionRatio = (1.0 - (double)totalEncodedBits / originalBits) * 100;
            cout << "Estimated compression ratio: " << compressionRatio << "%" << endl;
        }
    }

    const unordered_map<char, string>& getCodeMap() const {
        return codeMap;
    }
    
    const unordered_map<string, char>& getReverseCodeMap() const {
        return reverseCodeMap;
    }
};

// ==================== HuffCode 类型（基于 Bitmap） ====================
class HuffCode {
private:
    Bitmap bitmap;
    int length;
    
public:
    HuffCode() : bitmap(1024), length(0) {}
    
    void fromString(const string& binaryStr) {
        length = binaryStr.length();
        bitmap = Bitmap(length + 8); // 分配足够空间
        
        for (size_t i = 0; i < binaryStr.length(); i++) {
            if (binaryStr[i] == '1') {
                bitmap.set(i);
            } else {
                bitmap.clear(i);
            }
        }
    }
    
    string toString() const {
        return bitmap.bits2string(length);
    }
    
    void print() const {
        cout << toString() << endl;
        bitmap.printStats();
    }
    
    int getLength() const {
        return length;
    }
};

// ==================== 主程序 ====================
int main() {
    string sampleText = "fivescoreyearsagoagreatamericaninwhosesymbolicshadowwestandtodaysignedtheemancipationproclamationthismomentousdecreecameasagreatbeaconlightofhopetomillionsofnegroslaveswhohadbeensearedintheflamesofwitheringinjusticeitcameasajoyousdaybreaktotheendthelongnightoftheircaptivitybutonehundredyearslaterthenegrostillisnotfreeonehundredyearslaterthelifeofthenegroisstillsadlycrippledbythemanaclesofsegregationandthechainsofdiscriminationonehundredyearslaterthenegrolivesonalonelyislandofpovertyinthemidstofavastoceanofmaterialprosperityonehundredyearslaterthenegroisstilllanguishedinthecornersofamericansocietyandfindshimselfanexileinhisownlandandsowevecomeheretodaytodramatizeashamefulconditioninasensewevecometoournationscapitaltocashacheckwhenthearchitectsofourrepublicwrotethemagnificentwordsoftheconstitutionandthedeclarationofindependencetheyweresigningapromissorynotetowhicheveryamericanwastofallheirthisnotewasapromisethatallmenyesblackmenaswellaswhitemenwouldbeguaranteedtheunalienablerightsoflifelibertyandthepursuitofhappinessitisobvioustodaythatamericahasdefaultedonthispromissorynoteinsofarashercitizensofcolorareconcernedinsteadofflavoringthissacredobligationamericahasgiventhenegropeopleabadcheckacheckwhichhascomebackmarkedinsufficientfundsbutwerefusetobelievethatthebankofjusticeisbankruptwerefusetobelievethatthereareinsufficientfundsinthegreatvaultsofopportunityofthisnationandsowevecometocashthischeckacheckthatwillgiveusupondemandtherichesoffreedomandthesecurityofjusticewehavealsocometothishallowedspottoremindamericaofthefierceurgencyofnowthisisnotimetoengageintheluxuryofcoolingoffortotakethetranquilizingdrugofgradualismnowisthetimetomakerealt hepromisesofdemocracynowisthetimetorisefromthedarkanddesolatevalleyofsegregationtothesunlitpathofracialjusticenowisthetimetoliftournationfromthequicksandsofracialinjusticetothesolidrockofbrotherhoodnowisthetimetomakejusticearealityforallofgodschild renitwouldbefatalforthenationtooverlooktheurgencyofthemomentthisswelteringsummerofthenegroslegitimatediscontentwillnotpassuntilthereisaninvigoratingautumnoffreedomandequalitynineteensixt ythreeisnotanendbutabeginningandthosewhohopethatthenegroneededtoblowoffsteamandwillnowbecontentwillhavearudeawakeningifthenationreturnstobusinessasusualandtherewillbeneitherre stnortranquilityinamericauntilthenegroisgrantedhiscitizenshiprightsthewhirlwindsofrevoltwillcontinuetoshakethefoundationsofournationuntilthebrightdayofjusticeemergesbutthereissomethingth atimustsaytomypeoplewhostandonthewarmthresholdwhichleadsintothepalaceofjusticeintheprocessofgainingourrightfulplacewemustnotbeguiltyofwrongfuldeedsletusnotseektosatisfyourt hirstforfreedombydrinkingfromthecupofbitternessandhatredwemustforeverconductourstruggleonthehighplaneofdignityanddisciplinewemustnotallowourcreativeprotesttodegenerateintophysica lviolenceagainandagainwemustrisetothemajesticheightsofmeetingphysicalforcewithsoulforcethemarvelousnewmilitancywhichhasengulfedthenegrocommunitymustnotleadustoadistrustofallwh itepeopleformanyofourwhitebrothersasevidencedbytheirpresenceheretodayhavecometorealizethattheirdestinyistiedupwithourdestinyandtheyhavecometorealizethattheirfreedomisinextricablyboundtoourfreedomwecannotwalkaloneandaswewalkwemustmakethepledgethatweshallalwaysmarchaheadwecannotturnbacktherearethosewhoareaskingthedevoteesofcivilrightswhenwillyoubesatisfiedwecanneverbesatisfiedaslongasthenegroisthevictimoftheunspeakablehorrorsofpolicebrutalitywecanneverbesatisfiedaslongasourbodiesheavywiththefatigueoftravelcannotgainlodginginthemotelsofthehighwaysandthehotelsofthecitieswecannotbesatisfiedaslongasthenegrosbasicmobilityisfromasmallerghetttoalargeronewecanneverbesatisfiedaslongasourchildrenarestrippedoftheirselfhoodandrobbedoftheirdignitybysignsstatingforwhitesonlywecannotbesatisfiedaslongasanegroinmississippicannotvoteandanegroinnewyorkbelieveshehasnothingforwhichtovotenonowearenotsatisfiedandwewillnotbesatisfieduntiljusticerollsdownlikewatersandrighteousnesslikeamightystreamiamnotunmindfulthatsomeofyouhavecomehereoutofgreattrialsandtribulationssomeofyouhavecomefreshfromnarrowjailcellsandsomeofyouhavecomefromareaswhereyourquestquestforfreedomleftyoubatteredbythestormsofpersecutionandstaggeredbythewindsofpolicebrutalityyouhavebeentheveteransofcreativesufferingcontinuetoworkwiththefaiththatunearnedsufferingisredemptivegobacktomississippigobacktoalabamagobacktosouthcarolinagobacktogeorgiagobacktolouisianagobacktotheslumsandghetto sofournortherncitiesknowingthatsomehowthissituationcanandwillbechangedletusnotwallowinthevalleyofdespairisaytoyoutodaymyfriendsands oeventhoughwefacethedifficultiesoftodayandtomorrowistillhaveadreamitisadreamdeeplyrootedintheamericandreamihaveadreamthatonedaythisnationwillriseupandliveoutthetruemeaningofitscreedweholdthesetruthstobeselfevidentthatallmenarecreatedequalihaveadreamthatonedayontheredhillsofgeorgiathesonsofformerslavesandthesonsofformerslaveownerswillbeabletositdowntogetheratthetableofbrotherhoodihaveadreamthatonedayeventhestateofmississippiastateswelteringwiththeheatofinjusticeswelteringwiththeheatofoppressionwillbetransformedintoanoasisoffreedomandjusticeihaveadreamthatmyfourlittlechildrenwillonedayliveinationwheretheywillnotbejudgedbythecoloroftheirskinbutbythecontentoftheircharacterihaveadreamtodayihaveadreamthatonedaydowninalabamawithitsviciousracistswithitsgovernorhavinghislipsdrippingwiththewordsofinterpositionandnullificationonedayrightthereinalabamalittleblackboysandblackgirlswillbeabletojoinhandswithlittlewhiteboysandwhitegirlsassistersandbrothersihaveadreamtodayihaveadreamthatonedayeveryvalleyshallbeexaltedandeveryhillandmountainshallbemadelowtheroughplaceswillbemadeplainandthecrookedplaceswillbemadestraightandthegloryofthelordshallberevealedandallfleshshallseeittogetherthisisourhopeandthisisthefaiththatigobacktothesouthwithwiththisfaithwewillbeabletohewoutofthemountainofdespairastoneofhopewiththisfaithwewillbeabletotransformthejanglingdiscordsofournationintoabeautifulsymphonyofbrotherhoodwiththisfaithwewillbeabletoworktogethertopraytogethertostruggletogethertogotojailtogethertostandupforfreedomtogetherknowingthatwewillbefreeonedayandthiswillbethedaythiswillbethedaywhenallofgodschildrenwillbeabletosingwithnewmeaningmycountrytisoftheesweetlandoflibertyoftheeisinglandwheremyfathersdiedlandofthepilgrimspridefromeverymountainsideletfreedomringandifamericaistobeagreatnationthismustbecomesoands oletfreedomringfromtheprodigioushilltopsofnewhampshireletfreedomringfromthemightymountainsofnewyorkletfreedomringfromtheheighteningallegheniesofpennsylvanialetfreedomringfromthesnowcappedrockiesofcoloradoletfreedomringfromthecurvaceousslopesofcaliforniabutnotonlythatletfreedomringfromstonemountainofgeorgialetfreedomringfromlookoutmountainoftennesseeletfreedomringfromeveryhillandmolehillofmississippifromeverymountainsideletfreedomringandwhenthishappenswhenweallowfreedomringwhenweletitringfromeveryvillageandeveryhamleteverystateandeverycitywewillbeabletospeedupthatdaywhenallofgodschildrenblackmenandwhitemenjewsandgentilesprotestantsandcatholicswillbeabletojoinhandsandsinginthewordsoftheoldnegrospiritualfreeatlastfreeatlastthankgodalmightywearefreeatlast";
    
    cout << "Huffman Encoding on 'I Have a Dream' Speech" << endl;
    cout << "===========================================" << endl;
    
    // 显示文本统计信息
    int totalChars = 0;
    int alphaChars = 0;
    for (char ch : sampleText) {
        totalChars++;
        if (isalpha(ch)) alphaChars++;
    }
    cout << "Text Statistics:" << endl;
    cout << "Total characters: " << totalChars << endl;
    cout << "Alphabetic characters: " << alphaChars << endl;
    cout << "Non-alphabetic characters: " << (totalChars - alphaChars) << endl << endl;
    
    // 1. 构建 Huffman 树
    HuffTree huffTree;
    huffTree.buildFromText(sampleText);
    
    // 2. 显示编码表
    huffTree.printCodes();
    
    // 3. 编码一些关键单词
    vector<string> keyWords = {"dream", "freedom", "justice", "hope", "equality", "brotherhood"};
    
    cout << "\n\nEncoding Key Words:" << endl;
    cout << "===================" << endl;
    
    for (const auto& word : keyWords) {
        string encoded = huffTree.encode(word);
        cout << "Word: '" << word << "'" << endl;
        cout << "Encoded: " << encoded << endl;
        cout << "Length: " << encoded.length() << " bits" << endl;
        
        // 使用 HuffCode（基于 Bitmap）存储编码
        HuffCode huffCode;
        huffCode.fromString(encoded);
        cout << "Bitmap representation: ";
        huffCode.print();
        cout << endl;
    }
    
    // 4. 演示解码
    cout << "\n\nDecoding Demonstration:" << endl;
    cout << "======================" << endl;
    
    string testWord = "dream";
    string encodedWord = huffTree.encode(testWord);
    string decodedWord = huffTree.decode(encodedWord);
    
    cout << "Original word: '" << testWord << "'" << endl;
    cout << "Encoded: " << encodedWord << endl;
    cout << "Decoded: '" << decodedWord << "'" << endl;
    
    // 5. 编码整个演讲的部分（前100个字符作为示例）
    cout << "\n\nEncoding first 100 characters of the speech:" << endl;
    cout << "===========================================" << endl;
    
    string samplePart = sampleText.substr(0, 100);
    string encodedPart = huffTree.encode(samplePart);
    cout << "Original (first 100 chars): " << samplePart << endl;
    cout << "Encoded length: " << encodedPart.length() << " bits" << endl;
    cout << "Original would be: " << (100 * 8) << " bits (assuming 8-bit ASCII)" << endl;
    cout << "Compression achieved: " 
         << (1.0 - (double)encodedPart.length() / (100 * 8)) * 100 << "%" << endl;
    
    return 0;
}