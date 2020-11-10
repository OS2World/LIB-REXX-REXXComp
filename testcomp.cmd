/* test file form REXXCOMP.DLL */

rc = 0

ADDRESS CMD "erase test.cmp test.new"
call RxFuncAdd 'LoadCompressionFuncs', 'REXXCOMP', 'LoadCompressionFuncs'
call LoadCompressionFuncs

 rc = Compress('test.exe','test.cmp')

say 'Compress RC = ' rc

rc = TestCompressedFile('test.cmp')

say 'Test RC = ' rc

rc = DeCompress('test.cmp','test.new')

say 'DeCompress RC = ' rc

call UnLoadCompressionFuncs

exit
