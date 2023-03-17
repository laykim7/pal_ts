

Dim wOrg As Worksheet
Dim wEdit As Worksheet
Dim i As Integer
Dim j As Integer
Dim retV As Integer

Dim offX As Integer
Dim offY As Integer
Dim spX As Integer
Dim spY As Integer

Dim ipMax As Integer
Dim templateP As Integer
Dim k As Integer
Dim kEnd As Integer

Const ptX_max As Integer = 100
Dim ptX(ptX_max) As Integer
Dim ptXend As Integer

Dim SS As String
Dim tmpStr As String


Function gen_TimingRegister(offX_HD As Integer, offY_HD As Integer, offY_HDN As Integer)

    '======== file : TimingRegister.reg ==============================================================
    Dim strPath As String
    strPath = ThisWorkbook.Path & Application.PathSeparator & "TimingRegister" & devName & ".reg"
    Open strPath For Output As #1
    
    'Dim offX_HD, offY_HD As Integer     'header xy
    ptXend = 10

    Dim sSpace As String
    sSpace = ","

    '======== print header
    For i = 1 To ptX_max
        ptX(i) = 0
    Next i

    For spX = offX_HD To spX + 100
        If (wOrg.Cells(offY_HD, spX) = "begin") Then
            Exit For
        End If
    Next spX
    
    ipMax = wOrg.Cells(6, 3) 'IP MAX Value pointX
    templateP = wOrg.Cells(6, 4) 'IP MAX Value pointX
    ' MsgBox ipMax
    spX = spX + 1

    For i = spX To ptX_max
        If (wOrg.Cells(offY_HD, i) = "end") Then
            Exit For
        End If

        If (wOrg.Cells(offY_HD, i) <> "") Then
            ptX(i) = wOrg.Cells(offY_HD + offY_HDN, i)
            Print #1, "##" + wOrg.Cells(offY_HD, i) + "##" + sSpace;
        End If
    Next i
    Print #1, ""
    
    ptXend = i

    '======== print data
    For spY = offY_HD To offY_HD + 100
        If (wEdit.Cells(spY, offX_HD) = "begin") Then
            Exit For
        End If
    Next spY
    
    spY = spY + 1
    
    For i = spY To 30000
        If (wEdit.Cells(i, offX_HD) = "end") Then
            Exit For
        End If

        If (wEdit.Cells(i, offX_HD) <> "") Then
            If (wEdit.Cells(i, ipMax) = "") Then
              kEnd = 0
            Else
              kEnd = wEdit.Cells(i, ipMax)
            End If

              ' MsgBox kEnd

            For k = 0 To kEnd
                For j = 1 To ptXend
                    If (ptX(j) <> 0) Then
                        tmpStr = wEdit.Cells(i, ptX(j))
                        tmpStr = Replace(tmpStr, "@", k)
                        Print #1, tmpStr & sSpace;
                    End If
                Next j
                Print #1, ""
            Next k
        End If
    Next i

    Close #1
End Function
















Function gen_newTimingSub(offY_HD As Integer, offX_HD As Integer, offY_HDN As Integer)

    ptXend = 10

    Dim sSpace As String
    sSpace = ", "

    Dim sType As String
    sType = wOrg.Cells(offY_HD, offX_HD)

    '======== print header
    'file "db/bi.template"{ pattern
    Print #1, "file " + SS + "db/" + sType + ".template" + SS + "{ pattern"

    For i = 1 To ptX_max
        ptX(i) = 0
    Next i

    For spX = offX_HD To spX + 100
        If (wOrg.Cells(offY_HD, spX) = "begin") Then
            Exit For
        End If
    Next spX
    
    spX = spX + 1

    Print #1, "{SYS, SUBSYS, DEV, ";
    For i = spX To ptX_max
        If (wOrg.Cells(offY_HD, i) = "end") Then
            Exit For
        End If

        If (wOrg.Cells(offY_HD, i) <> "") Then
            ptX(i) = wOrg.Cells(offY_HD + 31, i)
            Print #1, wOrg.Cells(offY_HD, i);
            If (wOrg.Cells(offY_HD, i + 1) <> "") Then
                Print #1, sSpace;
            End If
        End If


    Next i
    Print #1, "}"
    
    ptXend = i

    '======== print data
    For spY = 3 To 100
        If (wEdit.Cells(spY, 3) = "begin") Then
            Exit For
        End If
    Next spY
    
    spY = spY + 1
    
    For i = spY To spY + 2000
        If (wEdit.Cells(i, 3) = "end") Then
            Exit For
        End If

        If (wEdit.Cells(i, ipMax) = "") Then
          kEnd = 0
        Else
          kEnd = wEdit.Cells(i, ipMax)
        End If

        ' MsgBox kEnd

        For k = 0 To kEnd
          If (wEdit.Cells(i, templateP) = sType) Then '24:template name : longin, waveform...
              Print #1,"{NEW_SYS, NEW_SUB_SYS, NEW_DEV_N, ";
              For j = 1 To ptXend
                  If (ptX(j) <> 0) Then
                      tmpStr = wEdit.Cells(i, ptX(j))
                      tmpStr = Replace(tmpStr, "@", k)
                      Print #1, tmpStr;
                      If (ptX(j + 1) <> 0) Then
                          Print #1, sSpace;
                      End If
                  End If
              Next j
              Print #1, "}"
          End If
        Next k
    Next i
    Print #1, "}"
    Print #1, ""
    Print #1, ""
End Function


Function gen_newTiming(offX_HD As Integer, offY_HD As Integer)
    '======== file : TimingRegister.reg ==============================================================
    Dim strPath2 As String
    strPath2 = ThisWorkbook.Path & Application.PathSeparator & "new_timing.sub"
    Open strPath2 For Output As #1

    ' MsgBox strPath2

    Dim listN As Integer

    'device type 헤더 찾기. bi,bo,mbbi,...
    For listN = offY_HD To offY_HD + 30
        If (wOrg.Cells(listN, offX_HD) = "begin") Then
            Exit For
        End If
    Next listN

    
    listN = listN + 1
    ' MsgBox wOrg.Cells(listN, offX_HD)
    
    Dim devTypeListN As Integer
    For devTypeListN = listN To listN + 20
        If (wOrg.Cells(devTypeListN, offX_HD) = "end") Then
            Exit For
        End If

        If (wOrg.Cells(devTypeListN, offX_HD) <> "") Then
            retV = gen_newTimingSub(devTypeListN, offX_HD, 40)
        End If
    Next devTypeListN

    Close #1
End Function




Sub fileWrite()
    Application.ScreenUpdating = False  '화면 업데이트 (일시) 중지

    Dim sOrg As String
    Dim sEdit As String

    sOrg = "org"
    sEdit = "edit"
    
    SS = Chr(34)

    Set wOrg = Worksheets(sOrg)
    Set wEdit = Worksheets(sEdit)

    retV = gen_TimingRegister(3, 2, 1)
    retV = gen_newTiming(2, 10)

    ' Dim devN As Integer
    ' Dim offY_dev As Integer

    ' offY_dev = 5
    ' For devN = 4 To 100
    '     If (wOrg.Cells(offY_dev, devN) = "end") Then
    '         Exit For
    '     End If

    '     If (wOrg.Cells(offY_dev, devN) <> "") Then
    '         devRow = wOrg.Cells(offY_dev + 1, devN)
    '         devName = wOrg.Cells(offY_dev, devN)
    '         'MsgBox devRow
    '         retV = gen_TimingRegister(3, 2, 1)
    '         retV = gen_newTiming(2, 10, 40)
    '     End If
    ' Next devN



    'gen_TimingRegister(offX_HD As Integer, offY_HD As Integer, offY_HDN As Integer, sSpace As String, sOrg As String, sEdit As String)
    'retV = gen_TimingRegister(3, 2, 1)
    'retV = gen_newTiming(2, 10, 40)
    
End Sub











