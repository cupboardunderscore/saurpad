import datetime

def uglyfy(filepath: str):
    with open(filepath, "r") as file:
        pretty = file.read()

    ugly_list = []
    for line in pretty.split("\n"):
        for index in range(len(line)):
            if line[index] != " ":
                ugly_list.append(line[index:])
                break

    ugly = "".join(ugly_list)
    return ugly

build_no = 0

try:
    with open("test/v.txt") as f:
        build_no = int(f.readline()) + 1
except:
    build_no = 1
with open("test/v.txt", 'w+') as f:
    f.write(str(build_no))

version = str(build_no) + "_" + datetime.datetime.now().astimezone().strftime("%Y-%m-%dT%H:%M:%S%z")

defs = """
#define defaultname "saurpad"
#define namestyle "saur\'pad"
#define ver "0.1-{}"
#define mver "0.2"
#define hver "0.1"

#define SD_CS 10
#define SPI_MOSI 11
#define SPI_MISO 13
#define SPI_SCK 12

#define I2C_SDA 19
#define I2C_SCL 20

#define I2S_DOUT GPIO_NUM_17
#define I2S_BCLK GPIO_NUM_0
#define I2S_LRC GPIO_NUM_18

#define P0 0
#define P1 1
#define P2 2
#define P3 3
#define P4 4
#define P5 5
#define P6 6
#define P7 7
#define P10 8
#define P11 9
#define P12 10
#define P13 11
#define P14 12
#define P15 13
#define P16 14
#define P17 15

#define index_set PROGMEM R"rawliteral({})rawliteral"
#define index_ota PROGMEM R"rawliteral({})rawliteral"
#define index_css PROGMEM R"rawliteral({})rawliteral"
""".format(version, uglyfy("test/setupserver.html"), uglyfy("test/otaserver.html"), uglyfy("test/index.css"))
with open("include/defs.hpp", 'w+') as f:
    f.write(defs)
