var vm1 = new Vue({
    el: '#vm1',
    data: {
        message: 'hello'
    },
    // 计算属性:如果你不希望有缓存，请用方法来替代。计算属性是基于它们的响应式依赖进行缓存的.
    computed: {
        // revesr...的getter函数
        reverseMessage: function(){
            // this指向vm1实例
            return this.message.split('').reverse().join('')
        },
        // setter函数
        setMessage: function(newMessage){
            message = newMessage;
        }
    }
});

// 监听器示例
var watchExampleVM = new Vue({

    el: '#watch-example',
    data: {
        question: '',
        answer: 'I cannot answer you until you ask me.',
    },
    watch: {
        // 如果question发生变化,则会执行此函数
        question: function(newQuestion, oldQuestion){
            this.answer = "waiting for you to stop asking.."
            this.debouncedGetAnswer()
        }
    },
    created: function(){
    // `_.debounce` 是一个通过 Lodash 限制操作频率的函数。
    // 在这个例子中，我们希望限制访问 yesno.wtf/api 的频率  
    this.debouncedGetAnswer = _.debounce(this.getAnswer, 500);    
        // this.debouncedGetAnswer = _.debounce(this.answer, 500);
    },
    methods: {
        getAnswer: function(){
            if(this.question.indexOf('?') == -1){
                this.answer = 'Questions usually contain a question mark. ;-)'
                return
            }
            this.answer = 'thinking'
            var vm = this
            anxios.get('https://yesno.wtf/api')
            .then(function (response) {
                vm.answer = _.capitalize(response.data.answer)
            })
            .catch(function(error){
                vm.answer = "error..sorry" + error
            })
        },
    },
    
})