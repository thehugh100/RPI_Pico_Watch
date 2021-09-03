Write-Host "Trying to connect to smartwatch on COM6"
$port = new-Object System.IO.Ports.SerialPort COM6,115200,None,8,one
$port.DtrEnable = 1
$port.Handshake = 'XOnXOff'
$port.open()
$port.DiscardInBuffer()
$port.DiscardOutBuffer()
$port.Write(“time ”)
$d = Get-Date -UFormat '%S %M %H %d %u %m %Y'
$port.WriteLine($d)
$port.ReadLine()

$port.DiscardInBuffer()
$port.DiscardOutBuffer()

$StartDate=(GET-DATE)
$EndDate=[datetime]"08/08/2021 14:50"
$DD = NEW-TIMESPAN -Start $EndDate -End $StartDate
$port.Write(“moonphase ”)
$port.WriteLine($DD.TotalDays)
$port.ReadLine()

$port.DiscardInBuffer()
$port.DiscardOutBuffer()

$port.Close()